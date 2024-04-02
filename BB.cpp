#include <iostream>
#include <vector>
#include <cstddef>
#include <deque>
#include <queue>
#include <limits>
#include <list>
#include <algorithm>
#include <ctime>
#include <string>
#include <fstream>
#include <cstdlib>
using namespace std;

class Task {
public:
    enum Status {
        READY,
        ESTIMATE
    };
    Status status;
    int procID;
    int cpu;
    int arrivalTime;
    int StartTime;
    int execTime;
    int finishTime;
    int remain_node;
    int anchor_start;
    int getArrivalTime() {
        return arrivalTime;
    };
    Task(int procID, int arrivalTime, int execTime, int cpu) {
        this->procID = procID;
        this->arrivalTime = arrivalTime;
        this->execTime = execTime;
        this->cpu = cpu;
    };
};


list<Task> runTable;
list<Task>::iterator it;
int totalCPUNum = 128;

struct cmp {
    bool operator ()(Task a, Task b) {
        return (a.arrivalTime == b.arrivalTime) ? (a.status < b.status) :
        (a.arrivalTime < b.arrivalTime);
    };
};
 
class Node {
public:

    int lb1;
    int lb2;
    int lb;
    Node* parent;
    vector<Node*> children;
    vector<Task> readyTasks;
    vector<Task> scheduledTasks;
    Node(Node* parent, vector<Task> x, vector<Task> y);
    void computelb(Node* node, int mode, int speedup);
};


Node::Node(Node* parent, vector<Task> x, vector<Task> y) : parent(parent) {
    this->lb1 = 0;
    this->lb2 = 0;
    this->scheduledTasks = x;
    this->readyTasks = y;
}

void Node::computelb(Node* node, int mode, int speedup) {
    bool isCPUIdle;
    int usage = totalCPUNum, anchor_point, sum = 0, maximum = 0, t;
    for (int i = 0; i < node->scheduledTasks.size(); i++) {
        anchor_point = 0;
        isCPUIdle = true;
        for (it = runTable.begin(); it != runTable.end(); it++) {
            if ( (usage >= node->scheduledTasks[i].cpu) && ((*it).getArrivalTime() > anchor_point) ) {
                isCPUIdle = false;
            }
            if ( (*it).status == Task::READY ) {
                if (isCPUIdle) {
               // cout << "Running process " << (*it).procID << " Released resources " << (*it).cpu << " Terminated time " << (*it).getArrivalTime() << endl;
                anchor_point = (*it).getArrivalTime();
                usage += (*it).cpu;
               // cout << "Usage: " << usage << endl;
                }
            }
            else if ( (*it).status == Task::ESTIMATE ) {
                if ( (*it).remain_node < node->scheduledTasks[i].cpu ) {
                    usage = (*it).remain_node;
                    isCPUIdle = true;
                }
                else if ( (*it).remain_node >= node->scheduledTasks[i].cpu ) {
                    usage = (*it).remain_node;
                    (*it).remain_node -= node->scheduledTasks[i].cpu;
                }
            }
        }
        if (speedup == 0) {
            t = node->scheduledTasks[i].execTime*(0.3 + (0.7/node->scheduledTasks[i].cpu));
        }
        else{
            t = node->scheduledTasks[i].execTime / node->scheduledTasks[i].cpu;
        }
        Task *run = new Task(node->scheduledTasks[i].procID, anchor_point + t, t, node->scheduledTasks[i].cpu);
        run->status = Task::READY;
        runTable.push_back(*run);
        usage -= node->scheduledTasks[i].cpu;
        node->scheduledTasks[i].finishTime = anchor_point + t;
        if (mode == 0) {
            if (anchor_point + t > maximum) {
                maximum = anchor_point + t;
            }
        }
        else{
            maximum += node->scheduledTasks[i].finishTime;
        }
        
        Task *Proc = new Task(node->scheduledTasks[i].procID, anchor_point, t, node->scheduledTasks[i].cpu);
        Proc->status = Task::ESTIMATE;
        Proc->remain_node = usage;
        Proc->finishTime = anchor_point + t;
        runTable.push_back(*Proc);
        node->scheduledTasks[i].StartTime = anchor_point;
        /*
        cout << "Process " << node->scheduledTasks[i].procID << ":" << endl <<
        "Resource need: " << node->scheduledTasks[i].cpu << endl <<
        "Remain node: " << usage << endl <<
        "Submit time: " << node->scheduledTasks[i].getArrivalTime() << endl <<
        "Anchor point: " << anchor_point << endl <<
        "Terminated time: " << anchor_point + t << endl << endl;
        */
        runTable.sort(cmp());
    }
    if (mode == 0) {
        node->lb1 = maximum;
    }
    else{
        node->lb1 = maximum / node->scheduledTasks.size();
    }
    maximum = 0;
    
    if (node->readyTasks.size() > 0) {
        if (mode == 0) {
            for (int i = 0; i < node->readyTasks.size(); i++) {
                if (node->readyTasks[i].execTime > maximum) {
                    maximum = node->readyTasks[i].execTime;
                }
            }
            node->lb2 = maximum;
        }
        else{
            for (int t = 0; t < node->readyTasks.size(); t++) {
                for (int j = 0; j <= t; j++) {
                    maximum += node->readyTasks[j].execTime;
                }
            }
            maximum /= totalCPUNum;
            node->lb2 = maximum;
        }
    }
    else{
        node->lb2 = 0;
    }
    node->lb = node->lb1 + node->lb2;
    runTable.clear();
    /*
    cout << "Lb1: " << node->lb1 << endl;
    cout << "Lb2: " << node->lb2 << endl;
    cout << "Lb1+Lb2 = " << node->lb << endl;
    cout << "E............N............D" << endl << endl << endl;
    */
}


int main() {
    ifstream read("l_sdsc_sp2.swf.extracted", ios::in);
    string ch;
    int load_factor = 1;
    int count;
    int jobCount = 0;
    int test = 0, record = 0;
    int n[17];
    int lb;
    int turnAroundTime = 0;
    int mode, speedup;
    vector<Task> *alltasks = new vector<Task>;
    vector<Node*> leafNodes;
    Node *head = NULL;
    vector<Task> *scheduledTasks = new vector<Task>;
    vector<Task> *readyTasks = new vector<Task>;
    Node *root = new Node(head, *scheduledTasks, *readyTasks);
    Node *current = root;
    int best = 0;
    int finish = 0;
    cout << "Enter criteria(makespan(0) or Average turnaround time(1)): " << endl;
    cin >> mode;
    cout << "Enter speed up mode(Amadals law(0) or linear speed up(1)): " << endl;
    cin >> speedup;

    while ( getline(read,ch) ) {
        record++;
       // cout << "Record: " << record << endl;
        string a = "";
        count = 0;
            for (int i = 0; i < 17; i++) {
        while ( ch[count] == ' ' || ch[count] == '\t' || ch[count] == '\n' )
        count++;
        while ( ch[count] != ' ' && ch[count] != '\t' && ch[count] != '\n') {
                a += ch[count++];
            }
            n[i] = stoi(a);
            a = "";
            }
        if ( record >= 1 ) {
            if ( n[10] == 1 ) {
            test++;
            n[1] = 0;
            n[3] = (double)n[3]*load_factor/(0.3+(0.7/n[7]));
            Task *Proc = new Task(jobCount, n[1], n[3], n[7]);
            Proc->status = Task::READY;
            alltasks->push_back(*Proc);
            jobCount++;
            }
        }
        if ( test == 10 )
            break;
    }
 
    for (int i = 0; i < alltasks->size(); i++) {
        vector<Task> *scheduledTasks = new vector<Task>;
        vector<Task> *readyTasks = new vector<Task>;
        for (int k = 0; k < alltasks->size(); k++) {
            readyTasks->push_back((*alltasks)[k]);
        }
        scheduledTasks->push_back((*readyTasks)[i]);
        readyTasks->erase(readyTasks->begin()+i);
        for (int j = 1; j <= totalCPUNum; j++) {
        vector<Task> *v1 = new vector<Task>;
        vector<Task> *v2 = new vector<Task>;
        v1 = scheduledTasks;
        v2 = readyTasks;
        (*v1)[0].cpu = j;
        Node *Newnode = new Node(root,*v1, *v2);
        Newnode->computelb(Newnode, mode, speedup);
        root->children.push_back(Newnode);
        leafNodes.push_back(Newnode);
        }
    }
    
    
    cout << endl <<
    "The best solution of branch and bound order is: ";
    for (int i = 0; i < leafNodes[best]->scheduledTasks.size(); i++) {
        cout << leafNodes[best]->scheduledTasks[i].procID << "(cpu: " << leafNodes[best]->scheduledTasks[i].cpu << ")";
        if ( i <= leafNodes[best]->scheduledTasks.size()-2 ) {
            cout << " -> ";
        }
        if (leafNodes[best]->scheduledTasks[i].finishTime > finish) {
            finish = leafNodes[best]->scheduledTasks[i].finishTime;
        }
    }

    cout << endl << "Average turnaround time: " << current->lb << endl
    << "makespan: " << finish << endl;
    leafNodes.clear();
    return 0;
}
