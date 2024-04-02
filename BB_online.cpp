#include <iostream>
#include "Process.h"
#include "Event.h"
#include "Scheduler.h"
#include "BB_online.h"
#include <limits>
#include <algorithm>
#include <list>
using namespace std;

extern int totalCPUNum;
extern double sysCPU;
extern double system_clock;
extern vector<Process> procTable;
int f = 0;
extern list<Process> runTable;
extern list<Process>::iterator it;

struct cmp {
    bool operator ()(Process a, Process b) {
        return (a.arrivalTime == b.arrivalTime) ? (a.status < b.status) :
        (a.arrivalTime < b.arrivalTime);
    };
};

BB_online::BB_online(EventPriorityQueue *eventQueue) : Scheduler<ProcessQueue>(eventQueue) {
}

BB_online::~BB_online() {
    while (!waitQueue.empty()) {
        Process topProc = waitQueue.top();
        waitQueue.pop();
    }
}

Node::Node(Node* parent, vector<Process> x, vector<Process> y) : parent(parent) {
    this->lb1 = 0;
    this->lb2 = 0;
    this->scheduledTasks = x;
    this->readyTasks = y;
}

void Node::computelb(Node* node) {
    int t, usage, anchor_point, maximum = 0, sum = 0;
    bool isCPUIdle;
    for (int i = 0; i < node->scheduledTasks.size(); i++) {
        usage = 0;
        anchor_point = system_clock;
        isCPUIdle = true;
        for (it = runTable.begin(); it != runTable.end(); it++) {
            if ( (usage >= node->scheduledTasks[i].request) && ((*it).getArrivalTime() > anchor_point) ) {
                isCPUIdle = false;
            }
            if ( (*it).refresh >= f && (*it).getArrivalTime() >= system_clock ) {
            if ( (*it).status == Process::READY ) {
                if (isCPUIdle) {
                anchor_point = (*it).getArrivalTime();
                usage += (*it).request;
                }
            }
            else if ( (*it).status == Process::ESTIMATE ) {
                if ( (*it).remain_node < node->scheduledTasks[i].request) {
                    usage = (*it).remain_node;
                    isCPUIdle = true;
                }
                else if ( (*it).remain_node >= node->scheduledTasks[i].request ) {
                    usage = (*it).remain_node;
                    (*it).remain_node -= node->scheduledTasks[i].request;
                }
            }
            }
            else{
                runTable.erase(it);
            }
        }
        t = node->scheduledTasks[i].execTime*(0.3 + (0.7/node->scheduledTasks[i].request));
        Process *run = new Process(node->scheduledTasks[i].procId, anchor_point + t, t, node->scheduledTasks[i].request);
        run->status = Process::READY;
        run->refresh = f;
        runTable.push_back(*run);
        usage -= node->scheduledTasks[i].request;
        node->scheduledTasks[i].finishTime = anchor_point + t;
        maximum += node->scheduledTasks[i].finishTime;
        sum += node->scheduledTasks[i].arrivalTime;
        
        Process *Proc = new Process(node->scheduledTasks[i].procId, anchor_point, t, node->scheduledTasks[i].request);
        Proc->status = Process::ESTIMATE;
        Proc->refresh = f;
        Proc->remain_node = usage;
        Proc->finishTime = anchor_point + t;
        runTable.push_back(*Proc);
        node->scheduledTasks[i].StartTime = anchor_point;
        runTable.sort(cmp());
    }
    node->lb1 = (maximum - sum) / node->scheduledTasks.size();
    maximum = 0;
    sum = 0;
    if (node->readyTasks.size() > 0) {
        for (int t = 0; t < node->readyTasks.size(); t++) {
            for (int j = 0; j <= t; j++) {
                maximum += node->readyTasks[j].execTime;
            }
        }
        maximum /= sysCPU;
        node->lb2 = maximum;
    }
    else{
        node->lb2 = 0;
    }
    node->lb = node->lb1 + node->lb2;
    f++;
}

void BB_online::Branch_and_bound() {
       vector<Process> *alltasks = new vector<Process>;
       vector<Process> *scheduledTasks = new vector<Process>;
       vector<Process> *readyTasks = new vector<Process>;
       vector<Node*> leafNodes;
       Node *head = NULL;
       Node *root = new Node(head, *scheduledTasks, *readyTasks);
       Node *current = root;
       int lb;
       int best = 0;
       int finish = 0;
       while(!waitQueue.empty()) {
       Process waitProc = waitQueue.top();
       waitQueue.pop();
       cout << waitProc.procId << endl;
       alltasks->push_back(waitProc);
       }
       for (int i = 0; i < alltasks->size(); i++) {
           vector<Process> *scheduledTasks = new vector<Process>;
           vector<Process> *readyTasks = new vector<Process>;
           for (int k = 0; k < alltasks->size(); k++) {
               readyTasks->push_back((*alltasks)[k]);
           }
           scheduledTasks->push_back((*readyTasks)[i]);
           readyTasks->erase(readyTasks->begin()+i);
           for (int j = 1; j <= sysCPU; j++) {
           vector<Process> *v1 = new vector<Process>;
           vector<Process> *v2 = new vector<Process>;
           v1 = scheduledTasks;
           v2 = readyTasks;
           (*v1)[0].request = j;
           Node *Newnode = new Node(root,*v1, *v2);
           Newnode->computelb(Newnode);
           root->children.push_back(Newnode);
           leafNodes.push_back(Newnode);
           }
       }
       lb = std::numeric_limits<int>::max();
       for (int i = 0; i < leafNodes.size(); i++) {
           if (leafNodes[i]->lb < lb) {
               best = i;
               lb = leafNodes[i]->lb;
               current = leafNodes[i];
           }
       }
       while(current->readyTasks.size() > 0) {
           leafNodes.erase(leafNodes.begin() + best);
       for (int i = 0; i < current->readyTasks.size(); i++) {
           vector<Process> *scheduledTasks = new vector<Process>;
           vector<Process> *readyTasks = new vector<Process>;
           readyTasks = &current->readyTasks;
           scheduledTasks = &current->scheduledTasks;
           scheduledTasks->push_back((*readyTasks)[i]);
           readyTasks->erase(readyTasks->begin()+i);
           for (int j = 1; j <= sysCPU; j++) {
           vector<Process> *v1 = new vector<Process>;
           vector<Process> *v2 = new vector<Process>;
           v1 = scheduledTasks;
           v2 = readyTasks;
           (*v1)[v1->size()-1].request = j;
           Node *Newnode = new Node(current,*v1, *v2);
           Newnode->computelb(Newnode);
           leafNodes.push_back(Newnode);
           current->children.push_back(Newnode);
           }
       }
           lb = std::numeric_limits<int>::max();
           for (int i = 0; i < leafNodes.size(); i++) {
               if (leafNodes[i]->lb < lb) {
                   best = i;
                   lb = leafNodes[i]->lb;
                   current = leafNodes[i];
               }
           }
       }
       cout << endl <<
       "The best solution of branch and bound order is: ";
       for (int i = 0; i < leafNodes[best]->scheduledTasks.size(); i++) {
           leafNodes[best]->scheduledTasks[i].priority = i;
           waitQueue.push(leafNodes[best]->scheduledTasks[i]);
           cout << leafNodes[best]->scheduledTasks[i].procId << "(cpu: " << leafNodes[best]->scheduledTasks[i].request << ")";
           if ( i <= leafNodes[best]->scheduledTasks.size()-2 && leafNodes[best]->scheduledTasks.size() > 1 ) {
               cout << " -> ";
           }
           if (leafNodes[best]->scheduledTasks[i].finishTime > finish) {
               finish = leafNodes[best]->scheduledTasks[i].finishTime;
           }
       }
       cout << endl << "Average turnaround time: " << current->lb << endl
       << "makespan: " << finish << endl << endl;
       leafNodes.clear();
}


void BB_online::schedule() {
    while (!waitQueue.empty()) {
        Process waitProc = waitQueue.top();
        if ( totalCPUNum > 0 ) {
            if ( waitQueue.size() == 1 ) {
                waitQueue.pop();
                waitProc.request = totalCPUNum;
            }
            else if ( totalCPUNum >= waitProc.request ) {
                waitQueue.pop();
            }
            else{
                break;
            }
            waitProc.StartTime = system_clock;
            waitProc.execTime = waitProc.execTime*(0.3+(0.7/waitProc.request));
            Event *runEvent = new Event(Event::CPU_COMPLETION, waitProc.procId, system_clock + waitProc.execTime);
            eventQueue->push(runEvent);
            Process *runProc = new Process(waitProc.procId, system_clock + waitProc.execTime, waitProc.execTime, waitProc.request);
            runProc->status = Process::READY;
            runProc->refresh = std::numeric_limits<int>::max();
            runTable.push_back(*runProc);
            totalCPUNum -= waitProc.request;
            procTable[waitProc.procId].request = waitProc.request;
            procTable[waitProc.procId].execTime = waitProc.execTime;
            cout << "Process " << waitProc.procId << " back to process queue" <<
            endl << "Process: " << waitProc.procId <<
            endl << "Using cpu number: " << waitProc.request <<
            endl << "Remaining cpu number: " << totalCPUNum <<
            endl << "Start time: " << system_clock << endl << endl;
        }
        else{
            Branch_and_bound();
            break;
        }
    }
}
