#pragma once
using namespace std;

class Process {

	public:
        enum Status {
            READY,
            TERMINATED,
            WAITING,
            ESTIMATE
        };
        Status status;
        int priority;
        int turnAroundTime;
		int procId;
		int arrivalTime;
        int StartTime;
        int execTime;
		int finishTime;
        int waitTime;
        int request;
        int remain_node;
        int anchor_start;
        int reduce;
        int refresh;
		int getArrivalTime();
        double efficiency;
        double alpha;
        bool selected;
        Process(int procId, int arrivalTime, int execTime, int request);
};

class LargestExecFirst {
    public:
        bool operator ()(Process a, Process b);
};

class SmallestFirst {
	public:
		bool operator ()(Process a, Process b);
};

class LargestFirst {
    public:
        bool operator ()(Process a, Process b);
};

class FCFS {
    public:
        bool operator ()(Process a, Process b);
};

class priority {
    public:
        bool operator ()(Process a, Process b);
};