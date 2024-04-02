#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <list>
#include "Event.h"
#include "SchedulerPointer.h"
#include "Scheduler_Utilization_Based.h"
#include "list_online.h"
#include "BB_online.h"
#include "MCTS_online.h"
#include "Hungarian_online.h"
using namespace std;

double score = 1000000;
vector<Process> scheduledTasks;
int totalCPUNum = 32;
int mode_select;
double sysCPU = 32;
double system_clock;
double waitingTimeSum = 0;
vector<Process> procTable;
list<Process> runTable;
list<Process> s_list;
list<Process> efficiency_list;
list<Process>::iterator it;
list<Process>::iterator s;

SchedulerPointer* Moldable(EventPriorityQueue *eventQueue, int schedulerChoice) {
    switch(schedulerChoice) {
        case 1:
        mode_select = 0;
        cout << "You chose Utilization-based-allocation." << endl;
        return new Scheduler_Utilization_Based(eventQueue);
        case 2:
        mode_select = 1;
        cout << "You chose Utilization-based-allocation( Hungarian-version )." << endl;
        return new Scheduler_Utilization_Based(eventQueue);
        case 3:
        cout << "You chose Branch-and-bound." << endl;
        return new BB_online(eventQueue);
        case 4:
        cout << "You chose Monte-carlo-tree-search." << endl;
        return new MCTS_online(eventQueue);
        case 5:
        cout << "You chose Hungarian-online." << endl;
        return new Hungarian_online(eventQueue);
        case 6:
        cout << "You chose List-scheduling." << endl;
        return new list_online(eventQueue);
        default:
        cout << "Invalid scheduler. Aborting." << endl;
        exit(1);
    }
}

void deallocateEventQueue(EventPriorityQueue *eventQueue) {
	while (!eventQueue->empty()) {
		Event *topEvent = eventQueue->top();
		eventQueue->pop();
		delete topEvent;
	}
	delete eventQueue;
}

int main() {
    ifstream read("l_sdsc_sp2.swf.extracted", ios::in);
    string ch;
    int begin = std::numeric_limits<int>::max();
    int scheduleChoice;
    int load_factor = 1;
    int count;
    int jobCount = 0;
    int mode;
    int n[17];
    int test = 0, record = 0;
    double serviceTimeSum = 0;
    double turnAroundTime = 0;
    EventPriorityQueue *eventQueue = new EventPriorityQueue();
    SchedulerPointer *scheduler;
    while ( getline(read,ch) ) {
        record++;
        string a = "";
        count = 0;
            for (int i = 0; i < 17; i++) {
        while ( ch[count] == ' ' || ch[count] == '\t' || ch[count] == '\n' )
        count++;
        while ( ch[count] != ' ' && ch[count] != '\t' && ch[count] != '\n') {
                a += ch[count++];
            }
            n[i] = stoi(a);
            a = "";
        }
        if ( record >= 194 ) {
            if ( n[10] == 1 ) {
            test++;
            if ( n[1] < begin ) {
                begin = n[1];
            }
            n[3] = (double)n[3]*load_factor/(0.3+(0.7/n[7]));
            if ( n[7] > totalCPUNum ) {
                n[7] = totalCPUNum;
            }
            cout << "Job " << jobCount << " Serial time: " << n[3] << " record: " << record << endl;
            Event *arrivalEvent = new Event(Event::PROCESS_ARRIVAL, jobCount, n[1]-begin);
            eventQueue->push(arrivalEvent);
            Process *newProc = new Process(jobCount, n[1]-begin, n[3], n[7]);
            newProc->alpha = 0.7;
            newProc->finishTime = 0;
            newProc->priority = jobCount;
            newProc->refresh = 0;
            procTable.push_back(*newProc);
            jobCount++;
            }
        }
        if ( test == 10 )
            break;
        }
    cout << endl << "Test number: " << test << endl << "Total cpu number: " << totalCPUNum
    << endl << "Load factor: " << load_factor << endl << endl
    << "Available schedulers: " << endl
    << "01. Utilization-based-allocation" << endl
    << "02. Utilization-based-allocation( Hungarian-version )" << endl
    << "03. Branch-and-bound" << endl
    << "04. Monte-carlo-tree-search" << endl
    << "05. Hungarian-online" << endl
    << "06. List-scheduling" << endl
    << "Choose one scheduler to simulate: " << endl;
    cin >> scheduleChoice;
    scheduler = Moldable(eventQueue, scheduleChoice);
    
    while (!eventQueue->empty()) {
        Event *nextEvent = eventQueue->top();
        eventQueue->pop();
        scheduler->handleEvent(nextEvent);
        delete nextEvent;
    }
    
    for (int i = 0; i < jobCount; i++) {
        serviceTimeSum += procTable[i].execTime;
        if ( (procTable[i].finishTime - procTable[i].getArrivalTime()) <= 0 ) {
            cout << "Process " << procTable[i].procId << " < 0" << endl;
        }
        turnAroundTime += (procTable[i].finishTime - procTable[i].getArrivalTime());
    }
    cout << endl <<
    "Makespan: " << system_clock << endl <<
    "Average turn around time: " << turnAroundTime / jobCount << endl <<
    "CPU Utilization: " << (100*serviceTimeSum /(system_clock*sysCPU)) << "%" << endl;
    cout << "Record: " << record << endl;
	deallocateEventQueue(eventQueue);
	return 0;
}
