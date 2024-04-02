#include <iostream>
#include "Process.h"
#include "Event.h"
#include "Scheduler.h"
#include "Scheduler_Utilization_Based.h"
#include "Hungarian.h"
#include <limits>
#include <algorithm>
#include <list>
#include <iomanip>
using namespace std;

extern int mode_select;
extern int totalCPUNum;
extern double sysCPU;
extern double system_clock;
extern double waitingTimeSum;
extern vector<Process> procTable;
int i, id, t, T, execTime, n, request, r;
double u, current, shortest, best;
extern list<Process> runTable;
extern list<Process> s_list;
extern list<Process> efficiency_list;
extern list<Process>::iterator it;
extern list<Process>::iterator s;



Scheduler_Utilization_Based::Scheduler_Utilization_Based(EventPriorityQueue *eventQueue) : Scheduler<order>(eventQueue) {
    this->isCPUIdle = false;
    this->modified = false;
    this->usage = 0;
    this->anchor_point = 0;
    this->count = 0;
    this->jobCount = 0;
}

Scheduler_Utilization_Based::~Scheduler_Utilization_Based() {
    while (!waitQueue.empty()) {
        Process topProc = waitQueue.top();
        waitQueue.pop();
    }
}

void Scheduler_Utilization_Based::Find_request() {
    int min, value, need, x, size;
    double s, k, time, r;
    vector<Process> decided;
    vector<double> v;
    vector< vector<double> > costMatrix;

    for (it = runTable.begin(); it != runTable.end(); it++) {
        if ((*it).status == Process::ESTIMATE && (*it).getArrivalTime() > system_clock) {
            Process *Proc = new Process(procTable[(*it).procId].procId, system_clock, procTable[(*it).procId].execTime, procTable[(*it).procId].request);
            waitQueue.push(*Proc);
        }
    }
    size = waitQueue.size();
    
    int i = 0;
    vector<Process> cpu_decide[size];
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
    cout << endl << "cost: " << cost << endl;
    sort(decided.begin(), decided.end(), work());
    cout << "After List rescheduling: " << endl << endl;
    for (int y = 0; y < decided.size(); y++) {
        cout << "Process " << decided[y].procId << " -> " << y << "th" << "(" << decided[y].request << ")" << "\t";
        cout << "work: " << decided[y].execTime*decided[y].request << endl;
    }
    cout << endl << endl;
    for (i = 0; i < decided.size(); i++) {
        decided[i].alpha = 0.7;
        decided[i].finishTime = 0;
        s_list.push_back(decided[i]);
    }
    decided.clear();
    for (i = 0; i < size; i++) {
    cpu_decide[i].clear();
    }
}

void Scheduler_Utilization_Based::highest_efficiency() {
    for (s = s_list.begin(); s != s_list.end(); s++) {
        if ((*s).selected) {
            (*s).selected = false;
            (*s).efficiency = (1/(1-((*s).alpha)+((*s).alpha/((*s).request))))/((*s).request);
            efficiency_list.push_back(*s);
        }
        else{
            (*s).efficiency = 0;
        }
    }
    efficiency_list.sort(num_cmp());
}

void Scheduler_Utilization_Based::back() {
    for (it = runTable.begin(); (it != runTable.end()) && totalCPUNum > 0; it++) {
        if ( (*it).status == Process::ESTIMATE && (system_clock == (*it).getArrivalTime()) ) {
            cout << "Process " << (*it).procId << " back to process queue" << endl;
            for (s = s_list.begin(); s != s_list.end(); s++) {
                if ( (*s).procId == (*it).procId ) {
                    s_list.erase(s);
                    break;
                }
            }
            procTable[(*it).procId].StartTime = system_clock;
            procTable[(*it).procId].request = (*it).request;
            Event *runEvent = new Event(Event::CPU_COMPLETION, (*it).procId, system_clock + (*it).execTime);
            eventQueue->push(runEvent);
            Process *runProc = new Process((*it).procId, system_clock + (*it).execTime, (*it).execTime, (*it).request);
            runProc->status = Process::READY;
            runProc->refresh = std::numeric_limits<int>::max();
            runTable.push_back(*runProc);
            totalCPUNum -= (*it).request;
            cout << "Process: " << (*it).procId <<
            endl << "Using cpu number: " << (*it).request <<
            endl << "Remaining cpu number: " << totalCPUNum <<
            endl << "Start time: " << system_clock << endl << endl;
        }
        runTable.sort(cmp());
    }
}

double Scheduler_Utilization_Based::Utilization() {
    double sum = 0, utilize, turnAroundTimeSum = 0, job = 0;
    int last_time;
    for (i = 0; i < procTable.size(); i++) {
        if ( procTable[i].finishTime != 0 ) {
            job++;
            sum += procTable[i].execTime;
            turnAroundTimeSum += (procTable[i].finishTime-procTable[i].getArrivalTime());
        }
    }
    for (it = runTable.begin(); it != runTable.end(); it++) {
        if ( (*it).status == Process::READY ) {
        job++;
            sum += procTable[(*it).procId].execTime;
            turnAroundTimeSum += ((*it).getArrivalTime()-procTable[(*it).procId].getArrivalTime());
            last_time = (*it).getArrivalTime();
        }
    }
    turnAroundTimeSum /= job;
    cout << "Turnaround time: " << turnAroundTimeSum << endl;
    utilize = sum / (sysCPU*last_time);
    return utilize;
}

void Scheduler_Utilization_Based::MLS() {
    int t, r;
    runTable.sort(cmp());
    for (s = s_list.begin(); s != s_list.end(); s++) {
    usage = totalCPUNum;
    anchor_point = system_clock;
    isCPUIdle = true;
    for (it = runTable.begin(); it != runTable.end(); it++) {
        if ( (usage >= (*s).request) && ((*it).getArrivalTime() > anchor_point) ) {
        isCPUIdle = false;
    }
        if ( procTable[(*it).procId].finishTime == 0 ) {
            if ( (*it).refresh >= count ) {
                if ( (*it).status == Process::READY ) {
                if (isCPUIdle) {
                    cout << "Running process " << (*it).procId << " Released resources " << (*it).request << " Terminated time " << (*it).getArrivalTime() << endl;
                    anchor_point = (*it).getArrivalTime();
                    usage += (*it).request;
                    cout << "Usage: " << usage << endl;
                }
            }
            else if ( (*it).status == Process::ESTIMATE ) {
                if ( (*it).remain_node < (*s).request ) {
                    usage = (*it).remain_node;
                    isCPUIdle = true;
                }
            else if ( (*it).remain_node >= (*s).request ) {
                    usage = (*it).remain_node;
                    (*it).remain_node -= (*s).request;
                }
            }
        }
        else{
            runTable.erase(it);
        }
    }
    else{
        runTable.erase(it);
    }
        
        }
        r = (*s).request;
        t = procTable[(*s).procId].execTime;
        Process *run = new Process((*s).procId, anchor_point + t*(1-((*s).alpha)+((*s).alpha/r)), t*(1-((*s).alpha)+((*s).alpha/r)), (*s).request);
        run->status = Process::READY;
        run->refresh = count;
        runTable.push_back(*run);
        usage -= (*s).request;
        
        Process *Proc = new Process((*s).procId, anchor_point, t*(1-((*s).alpha)+((*s).alpha/r)), (*s).request);
        Proc->refresh = count;
        Proc->status = Process::ESTIMATE;
        Proc->remain_node = usage;
        Proc->finishTime = anchor_point + t*(1-((*s).alpha)+((*s).alpha/r));
        runTable.push_back(*Proc);
        
        cout << "Process " << (*s).procId << ":" << endl <<
        "Resource need: " << (*s).request << endl <<
        "Remain node: " << usage << endl <<
        "Submit time: " << (*s).getArrivalTime() << endl <<
        "Anchor point: " << anchor_point << endl <<
        "Terminated time: " << anchor_point + t*((1-((*s).alpha))+((*s).alpha/r)) << endl << endl;
        runTable.sort(cmp());
        }
}

void Scheduler_Utilization_Based::Two_Level_Allocation() {
        count++;
        MLS();
        u = Utilization();
        best = 0;
        int id;
        cout << "Initialized Utilization: " << 100 * u << "%" << endl << endl;
        do {
            pass = false;
            modified = false;
            current = u;
            if ( u >= best ) {
                record.clear();
                for (it = runTable.begin(); it != runTable.end(); it++) {
                    record.push_back(*it);
                }
                best = u;
            }
            else{
                break;
            }
            for (it = runTable.begin(); it != runTable.end(); it++) {
                if ( modified && ((*it).getArrivalTime() > anchor_point) ) {
                break;
                }
                if ( (*it).status == Process::ESTIMATE && ((*it).remain_node > 0) ) {
                    usage = (*it).remain_node;
                    anchor_point = (*it).getArrivalTime();
                    modified = true;
                    for (s = s_list.begin(); s != s_list.end(); s++) {
                        if ((*s).procId == (*it).procId) {
                            (*s).selected = true;
                            break;
                        }
                    }
                }
            }
            if (modified) {
                pass = true;
                cout << "Modified..." << endl;
                n = usage;
                highest_efficiency();
                while ( n > 0 ) {
                    for (it = efficiency_list.begin(); it != efficiency_list.end(); it++) {
                    for (s = s_list.begin(); s != s_list.end(); s++) {
                        if ( ((*s).procId == (*it).procId) && n > 0 ) {
                            (*s).request++;
                            n--;
                        }
                    }
                    }
                }
                efficiency_list.clear();
                count++;
                MLS();
                u = Utilization();
                cout << "Utilization: " << 100 * u << "%" << endl << endl;
            }
            else{
                for (it = runTable.begin(); it != runTable.end(); it++) {
                    if ((*it).status == Process::READY) {
                        id = (*it).procId;
                    }
                }
                for (s = s_list.begin(); s != s_list.end(); s++) {
                    if ( (*s).procId == id && (*s).request < sysCPU ) {
                    pass = true;
                        (*s).request++;
                    break;
                    }
                }
                if (pass) {
                cout << "Unmodified..." << endl;
                count++;
                MLS();
                u = Utilization();
                cout << "Utilization: " << 100 * u << "%" << endl << endl;
                }
            }
        } while(pass);
    cout << "Round: " << count << endl << endl;
    count++;
    for (s = s_list.begin(); s != s_list.end(); s++) {
        (*s).request = 1;
    }
    runTable.clear();
    for (i = 0; i < record.size(); i++) {
        runTable.push_back(record[i]);
    }
    record.clear();
    if (mode_select == 1) {
    s_list.clear();
    }
}

void Scheduler_Utilization_Based::schedule() {
    back();
    while (!waitQueue.empty()) {
        Process waitProc = waitQueue.top();
        waitQueue.pop();
        if ( totalCPUNum > 0 ) {
            if ( waitQueue.size() == 0 ) {
                waitProc.request = totalCPUNum;
            }
            waitProc.execTime = waitProc.execTime*(0.3+(0.7/waitProc.request));
            procTable[waitProc.procId].request = waitProc.request;
            procTable[waitProc.procId].execTime = waitProc.execTime;
            totalCPUNum -= waitProc.request;
            Event *runEvent = new Event(Event::CPU_COMPLETION, waitProc.procId, system_clock + waitProc.execTime);
            eventQueue->push(runEvent);
            Process *runProc = new Process(waitProc.procId, system_clock + waitProc.execTime, waitProc.execTime, waitProc.request);
            runProc->status = Process::READY;
            runProc->refresh = std::numeric_limits<int>::max();
            runTable.push_back(*runProc);
            cout << "Process " << waitProc.procId << " back to process queue" <<
            endl << "Process: " << waitProc.procId <<
            endl << "Using cpu number: " << waitProc.request <<
            endl << "Remaining cpu number: " << totalCPUNum <<
            endl << "Start time: " << system_clock << endl << endl;
        }
        else{
            if (mode_select == 0) {
            waitProc.request = 1;
            s_list.push_back(waitProc);
            }
            int t = std::numeric_limits<int>::max();
            Process *Proc = new Process(waitProc.procId, t, waitProc.execTime, waitProc.request);
            Proc->status = Process::ESTIMATE;
            Proc->refresh = count;
            runTable.push_back(*Proc);
            if (mode_select == 1) {
            Find_request();
            }
            Two_Level_Allocation();
        }
    }
}
