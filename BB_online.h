#include "Scheduler.h"
using namespace std;

class BB_online : public Scheduler<ProcessQueue> {

    public:
        BB_online(EventPriorityQueue *eventQueue);
        virtual ~BB_online();
        void schedule();
        void Branch_and_bound();
};

class Node {
public:

    int lb1;
    int lb2;
    int lb;
    Node* parent;
    vector<Node*> children;
    vector<Process> readyTasks;
    vector<Process> scheduledTasks;
    Node(Node* parent, vector<Process> x, vector<Process> y);
    void computelb(Node* node);
};
