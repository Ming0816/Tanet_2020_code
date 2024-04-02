#pragma once
#include "Scheduler.h"
using namespace std;

class list_online : public Scheduler<order> {

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
        struct work {
            bool operator ()(Process a, Process b) {
                return (a.execTime*a.request == b.execTime*b.request) ? (a.procId < b.procId) :
                (a.execTime*a.request < b.execTime*b.request);
            };
        };
        
        bool modified, pass;
        bool isCPUIdle;
        int usage, count, jobCount, i, id, t, T, execTime, n, request, r;
        double anchor_point, u, current, shortest, best;
        vector<Process> record;
        list_online(EventPriorityQueue *eventQueue);
        virtual ~list_online();
        void schedule();
        double Utilization();
        void highest_efficiency();
        void MLS();
        void back();
        void Two_Level_Allocation();
};
