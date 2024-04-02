#pragma once
#include "Scheduler.h"
using namespace std;

class Hungarian_online : public Scheduler<ProcessQueue> {

    public:
        Hungarian_online(EventPriorityQueue *eventQueue);
        virtual ~Hungarian_online();
        void schedule();
        void Hungarian_solve();
};
