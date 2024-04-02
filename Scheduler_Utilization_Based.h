#pragma once
#include "Scheduler.h"
using namespace std;

class Scheduler_Utilization_Based : public Scheduler<order> {

    public:
        struct cmp {
            bool operator ()(Process a, Process b) {
                return (a.arrivalTime == b.arrivalTime) ? (a.status < b.status) :
                (a.arrivalTime < b.arrivalTime);
            };
        };
        struct num_cmp {
            bool operator ()(Process a, Process b) {
                return (a.efficiency == b.efficiency) ? (a.procId < b.procId) :
                (a.efficiency > b.efficiency);
            };
        };
        struct priority {
            bool operator ()(Process a, Process b) {
                return (a.priority < b.priority);
            };
        };
        struct work {
            bool operator ()(Process a, Process b) {
                return a.execTime*a.request < b.execTime*b.request;
            };
        };
        
        bool modified, pass;
        bool isCPUIdle;
        int usage, count, jobCount;
        double anchor_point;
        vector<Process> record;
        Scheduler_Utilization_Based(EventPriorityQueue *eventQueue);
        virtual ~Scheduler_Utilization_Based();
        void schedule();
        double Utilization();
        void highest_efficiency();
        void MLS();
        void back();
        void Two_Level_Allocation();
        void Find_request();
};
