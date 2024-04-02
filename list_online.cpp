#include <iostream>
#include "Process.h"
#include "Event.h"
#include "Scheduler.h"
#include "list_online.h"
#include <limits>
#include <algorithm>
#include <list>
using namespace std;

extern int totalCPUNum;
extern double sysCPU;
extern double system_clock;
extern double waitingTimeSum;
extern vector<Process> procTable;
extern list<Process> runTable;
extern list<Process> s_list;
extern list<Process> efficiency_list;
extern list<Process>::iterator it;
extern list<Process>::iterator s;

list_online::list_online(EventPriorityQueue *eventQueue) : Scheduler<order>(eventQueue) {
    this->isCPUIdle = false;
    this->modified = false;
    this->usage = 0;
    this->anchor_point = 0;
    this->count = 0;
    this->jobCount = 0;
}

list_online::~list_online() {
    while (!waitQueue.empty()) {
        Process topProc = waitQueue.top();
        waitQueue.pop();
    }
}

void list_online::highest_efficiency() {
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

void list_online::back() {
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

double list_online::Utilization() {
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

void list_online::MLS() {
    int t, r;
    s_list.sort(work());
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

void list_online::Two_Level_Allocation() {
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
}

void list_online::schedule() {
    back();
    while (!waitQueue.empty()) {
        Process waitProc = waitQueue.top();
        waitQueue.pop();
        if ( totalCPUNum > 0 ) {
            cout << "Process " << waitProc.procId << " back to process queue" << endl;
            T = waitProc.execTime * ((1-waitProc.alpha)+(waitProc.alpha/totalCPUNum));
            procTable[waitProc.procId].request = waitProc.request = totalCPUNum;
            totalCPUNum = 0;
            Event *runEvent = new Event(Event::CPU_COMPLETION, waitProc.procId, system_clock + T);
            eventQueue->push(runEvent);
            Process *runProc = new Process(waitProc.procId, system_clock + T, T, waitProc.request);
            runProc->status = Process::READY;
            runProc->refresh = std::numeric_limits<int>::max();
            runTable.push_back(*runProc);
            cout << "Process: " << waitProc.procId <<
            endl << "Using cpu number: " << waitProc.request <<
            endl << "Remaining cpu number: " << totalCPUNum <<
            endl << "Start time: " << system_clock << endl << endl;
        }
        else{
            waitProc.request = 1;
            s_list.push_back(waitProc);
            Two_Level_Allocation();
        }
    }
}
