#pragma once
#include "Event.h"
using namespace std;

class SchedulerPointer {
	public:

		virtual void handleEvent(Event *event) = 0;
};
