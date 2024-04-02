#include<iostream>
#include "Process.h"
using namespace std;

Process::Process(int procId, int arrivalTime, int execTime, int request) {
    this->procId = procId;
    this->arrivalTime = arrivalTime;
    this->execTime = execTime;
    this->request = request;
}

bool LargestExecFirst::operator ()(Process a, Process b) {
    return (a.execTime == b.execTime) ? (a.procId > b.procId) :
    (a.execTime < b.execTime);
}

bool SmallestFirst::operator ()(Process a, Process b) {
    return (a.request == b.request) ? (a.arrivalTime > b.arrivalTime) : (a.request > b.request);
}

bool LargestFirst::operator ()(Process a, Process b) {
    return (a.request == b.request) ? (a.arrivalTime > b.arrivalTime) :    (a.request < b.request);
}

bool FCFS::operator ()(Process a, Process b) {
    return (a.arrivalTime == b.arrivalTime) ? (a.procId > b.procId) :
    (a.arrivalTime > b.arrivalTime);
}

bool priority::operator ()(Process a, Process b) {
    return (a.priority > b.priority);
}

int Process::getArrivalTime() {
	return arrivalTime;
}
