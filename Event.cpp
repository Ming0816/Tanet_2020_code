#include<iostream>
#include "Event.h"
using namespace std;

Event::Event(EventType eventType, int procId, double time) {
    this->eventType = eventType;
    this->procId = procId;
    this->time = time;
}

bool EventComparator::operator ()(Event *a, Event *b) {
	return a->time > b->time;
}
