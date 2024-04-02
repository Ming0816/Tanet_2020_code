#include <iostream>
#include "Process.h"
#include "Event.h"
#include "Scheduler.h"
#include "Hungarian_online.h"
#include "Hungarian.h"
#include <limits>
#include <vector>
#include <algorithm>
#include <list>
#include <iomanip>
using namespace std;

extern int totalCPUNum;
extern double sysCPU;
extern double system_clock;
extern vector<Process> procTable;

struct cmp {
    bool operator ()(Process a, Process b) {
        return (a.arrivalTime == b.arrivalTime) ? (a.status < b.status) :
        (a.arrivalTime < b.arrivalTime);
    };
};

struct work {
    bool operator ()(Process a, Process b) {
        return a.execTime*a.request < b.execTime*b.request;
    };
};

Hungarian_online::Hungarian_online(EventPriorityQueue *eventQueue) : Scheduler<ProcessQueue>(eventQueue) {
}

Hungarian_online::~Hungarian_online() {
    while (!waitQueue.empty()) {
        Process topProc = waitQueue.top();
        waitQueue.pop();
    }
}

void Hungarian_online::Hungarian_solve() {
    int min, value, need, x, size = waitQueue.size();
    double s, k, time, r;
    vector<Process> cpu_decide[size];
    vector<Process> decided;
    vector<double> v;
    vector< vector<double> > costMatrix;
    
    int i = 0;
    while (!waitQueue.empty()) {
        Process waitProc = waitQueue.top();
        waitQueue.pop();
        time = waitProc.execTime;
        for (s = 0; s < size; s++) {
            min = std::numeric_limits<int>::max();
            for (k = 1; k <= sysCPU; k++) {
                value = time*(0.3+(0.7/k)) + (2*(size-1-s)+1)*(k/sysCPU)*(time*(0.3+(0.7/k)));
                if (value < min) {
                    min = value;
                    need = k;
                }
            }
            Process *newProc = new Process(waitProc.procId, waitProc.getArrivalTime(), waitProc.execTime, need);
            newProc->finishTime = min;
            cpu_decide[i].push_back(*newProc);
        }
        i++;
    }
    
    for (i = 0; i < size; i++) {
        for (s = 0; s < size; s++) {
            time = cpu_decide[i][s].execTime;
            r = cpu_decide[i][s].request;
            x = time*(0.3+(0.7/r)) + (2*(size-1-s)+1)*(r/sysCPU)*(time*(0.3+(0.7/r)));
            v.push_back(x);
        }
       // cout << endl;
        costMatrix.push_back(v);
        v.clear();
    }
    HungarianAlgorithm HungAlgo;
    vector<int> assignment;
    double cost = HungAlgo.Solve(costMatrix, assignment);
    
    cout << "After Hungarian scheduling: " << endl << endl;
    for (int y = 0; y < costMatrix.size(); y++) {
        cout << "Process " << cpu_decide[y][assignment[y]].procId << " -> " << assignment[y] << "th" << "(" << cpu_decide[y][assignment[y]].request << ")" << "\t";
        cout << "work: " << cpu_decide[y][assignment[y]].request*cpu_decide[y][assignment[y]].execTime << endl;
        cpu_decide[y][assignment[y]].priority = assignment[y];
        if (cpu_decide[y][assignment[y]].request > (sysCPU/2) )
            cpu_decide[y][assignment[y]].request = (sysCPU/2);
        decided.push_back(cpu_decide[y][assignment[y]]);
    }
    cout << endl << "cost: " << cost << endl << endl;
    sort(decided.begin(), decided.end(), work());
    cout << "After List rescheduling: " << endl << endl;
    for (int y = 0; y < decided.size(); y++) {
        cout << "Process " << decided[y].procId << " -> " << y << "th" << "(" << decided[y].request << ")" << "\t";
        cout << "work: " << decided[y].execTime*decided[y].request << endl;
        decided[y].priority = y;
    }
    cout << endl << endl;
    for (i = 0; i < decided.size(); i++) {
        waitQueue.push(decided[i]);
    }
    decided.clear();
    for (i = 0; i < size; i++) {
    cpu_decide[i].clear();
    }
}


void Hungarian_online::schedule() {
    while (!waitQueue.empty()) {
        Process waitProc = waitQueue.top();
        if ( totalCPUNum > 0 ) {
            if ( waitQueue.size() == 1 ) {
                waitQueue.pop();
                waitProc.request = totalCPUNum;
            }
            else if ( totalCPUNum >= waitProc.request ) {
                waitQueue.pop();
            }
            else{
                break;
            }
            waitProc.execTime = waitProc.execTime*(0.3+(0.7/waitProc.request));
            procTable[waitProc.procId].request = waitProc.request;
            procTable[waitProc.procId].execTime = waitProc.execTime;
            totalCPUNum -= waitProc.request;
            Event *runEvent = new Event(Event::CPU_COMPLETION, waitProc.procId, system_clock + waitProc.execTime);
            eventQueue->push(runEvent);
            cout << "Process " << waitProc.procId << " back to process queue" <<
            endl << "Process: " << waitProc.procId <<
            endl << "Using cpu number: " << waitProc.request <<
            endl << "Remaining cpu number: " << totalCPUNum <<
            endl << "Start time: " << system_clock << endl << endl;
        }
        else{
        Hungarian_solve();
        break;
        }
    }
}
