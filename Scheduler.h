#pragma once
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <queue>
#include "Process.h"
#include "Event.h"
#include "SchedulerPointer.h"
using namespace std;

extern int totalCPUNum;
extern double system_clock;
extern vector<Process> procTable;

typedef priority_queue<Process, deque<Process>, priority> ProcessQueue;
typedef priority_queue<Process, deque<Process>, FCFS> order;
typedef priority_queue<Process, deque<Process>, LargestExecFirst> LGF;
typedef priority_queue<Process, deque<Process>, SmallestFirst> order_smallest;
typedef priority_queue<Process, deque<Process>, LargestFirst> order_largest;
typedef priority_queue<Event*, deque<Event*>, EventComparator> EventPriorityQueue;

template <typename WaitingQueue>
class Scheduler : public SchedulerPointer {
    protected:
		EventPriorityQueue *eventQueue;
        WaitingQueue waitQueue;
    
	public:
		Scheduler(EventPriorityQueue *eventQueue){
            this->eventQueue = eventQueue;
        };
    
		virtual ~Scheduler(){
            procTable.clear();
            eventQueue = NULL;
        };

        virtual void handleEvent(Event *event){
            system_clock = event->time;
            switch (event->eventType) {
                case Event::PROCESS_ARRIVAL:
                    waitQueue.push(procTable[event->procId]);
                    schedule();
                    break;
                case Event::CPU_COMPLETION:
                    totalCPUNum += procTable[event->procId].request;
                    procTable[event->procId].finishTime = system_clock;
                    cout << "Process " << event->procId << " completed..." <<
                    "Terminated time: " << system_clock << endl <<
                    "Released cpu number: " << procTable[event->procId].request << endl <<
                    "Available cpu number: " << totalCPUNum << endl << endl;
                    schedule();
                    break;
                default:
                    break;
            }
        };
    
        virtual void schedule() = 0;
};
