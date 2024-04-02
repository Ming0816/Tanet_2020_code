#include <iostream>
#include "Process.h"
#include "Event.h"
#include "Scheduler.h"
#include "MCTS_online.h"
#include <limits>
#include <algorithm>
#include <list>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace std;

extern int totalCPUNum;
extern double score;
extern double sysCPU;
extern double system_clock;
extern vector<Process> procTable;
extern vector<Process> scheduledTasks;
int frequent = 0;
extern list<Process> runTable;
extern list<Process>::iterator it;

struct cmp {
    bool operator ()(Process a, Process b) {
        return (a.arrivalTime == b.arrivalTime) ? (a.status < b.status) :
        (a.arrivalTime < b.arrivalTime);
    };
};

MCTS_online::MCTS_online(EventPriorityQueue *eventQueue) : Scheduler<ProcessQueue>(eventQueue) {
}

MCTS_online::~MCTS_online() {
    while (!waitQueue.empty()) {
        Process topProc = waitQueue.top();
        waitQueue.pop();
    }
}

TreeNode::TreeNode(TreeNode* parent, vector<Process> x, vector<Process> y) : parent(parent) {
    this->num_times = 1;
    this->mv = 0;
    this->sd = 0;
    this->score = 0;
    this->um = 0;
    this->pm = 0;
    this->scheduledTasks = x;
    this->readyTasks = y;
}

double TreeNode::MLS(TreeNode* node) {
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
            if ( (*it).refresh >= frequent && (*it).getArrivalTime() >= system_clock ) {
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
        run->refresh = frequent;
        runTable.push_back(*run);
        usage -= node->scheduledTasks[i].request;
        node->scheduledTasks[i].finishTime = anchor_point + t;
        maximum += node->scheduledTasks[i].finishTime;
        sum += node->scheduledTasks[i].arrivalTime;
        
        Process *Proc = new Process(node->scheduledTasks[i].procId, anchor_point, t, node->scheduledTasks[i].request);
        Proc->status = Process::ESTIMATE;
        Proc->refresh = frequent;
        Proc->remain_node = usage;
        Proc->finishTime = anchor_point + t;
        runTable.push_back(*Proc);
        node->scheduledTasks[i].StartTime = anchor_point;
        runTable.sort(cmp());
    }
    frequent++;
    return (maximum - sum) / node->scheduledTasks.size();
}

void MCTS_online::MCTS() {
    bool simulation_change = false;
    int times = 0;
    vector<Process> *alltasks = new vector<Process>;
    vector<TreeNode*> leafNodes;
    vector<TreeNode*> temp_list;
    TreeNode *head = NULL;
    vector<Process> *scheduled = new vector<Process>;
    vector<Process> *ready = new vector<Process>;
    TreeNode *current;
    TreeNode *root = new TreeNode(head, *scheduled, *ready);
    int length;
    if ( waitQueue.size() < 4 ) {
        simulation_change = true;
    }
    while(!waitQueue.empty()) {
    Process waitProc = waitQueue.top();
    waitQueue.pop();
    cout << waitProc.procId << endl;
    alltasks->push_back(waitProc);
    }
    if (!simulation_change) {
    root = new TreeNode(head, *scheduled, *alltasks);
    current = root;
    }
    else{
        current = root;
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
        TreeNode *Newnode = new TreeNode(root,*v1, *v2);
        root->children.push_back(Newnode);
        leafNodes.push_back(Newnode);
        }
    }
    for (int i = 0; i < 2; i++) {
        length = leafNodes.size();
        for (int j = 0; j < length; j++) {
            current = leafNodes[0];
            leafNodes.erase(leafNodes.begin());
            for (int m = 0; m < current->readyTasks.size(); m++) {
                vector<Process> *scheduledTasks = new vector<Process>;
                vector<Process> *readyTasks = new vector<Process>;
                readyTasks = &current->readyTasks;
                scheduledTasks = &current->scheduledTasks;
                scheduledTasks->push_back((*readyTasks)[m]);
                readyTasks->erase(readyTasks->begin()+m);
                for (int p = 1; p <= sysCPU; p++) {
                vector<Process> *v1 = new vector<Process>;
                vector<Process> *v2 = new vector<Process>;
                v1 = scheduledTasks;
                v2 = readyTasks;
                (*v1)[v1->size()-1].request = p;
                TreeNode *Newnode = new TreeNode(current,*v1, *v2);
                temp_list.push_back(Newnode);
                current->children.push_back(Newnode);
                }
            }
        }
        leafNodes = temp_list;
        temp_list.clear();
    }
    }
    srand(1);
    if (!simulation_change) {
    for (int i = 0; i < 1000; i++) {
        current = root;
        bool flag = false;
        times = 0;
        while (!flag) {
            if (times == 1)
            break;
            if (current->children.size() == 0){
                if (current->readyTasks.size() != 0)
                {
				 vector<Process> scheduledTasks;
                 vector<Process> readyTasks;
                 readyTasks = current->readyTasks;
                 scheduledTasks = current->scheduledTasks;
                 scheduledTasks.push_back(readyTasks[0]);
                 readyTasks.erase(readyTasks.begin()+0);
                 vector<Process> *v1 = new vector<Process>;
                 vector<Process> *v2 = new vector<Process>;
                 v1 = &scheduledTasks;
                 v2 = &readyTasks;
                 (*v1)[v1->size()-1].request = 1;
                 TreeNode *Newnode = new TreeNode(current,*v1, *v2);
                 current->children.push_back(Newnode);
                 current = Newnode;
                }
               flag = true;
            }
            else {
                if (current->children.size() < current->readyTasks.size()*sysCPU) {
                    for (int x = 0; x < current->readyTasks.size(); x++) {
                        for (int y = 1; y <= sysCPU; y++) {
                            for (int z = 0; z < current->children.size(); z++) {
                                if ( current->readyTasks[x].procId != current->children[z]->scheduledTasks[current->children[z]->scheduledTasks.size()-1].procId || current->readyTasks[x].request != current->children[z]->scheduledTasks[current->children[z]->scheduledTasks.size()-1].request) {
                                    vector<Process> scheduledTasks;
                                    vector<Process> readyTasks;
                                    readyTasks = current->readyTasks;
                                    scheduledTasks = current->scheduledTasks;
                                    scheduledTasks.push_back(readyTasks[x]);
                                    readyTasks.erase(readyTasks.begin()+x);
                                    vector<Process> *v1 = new vector<Process>;
                                    vector<Process> *v2 = new vector<Process>;
                                    v1 = &scheduledTasks;
                                    v2 = &readyTasks;
                                    (*v1)[v1->size()-1].request = y;
                                    TreeNode *Newnode = new TreeNode(current,*v1, *v2);
                                    current->children.push_back(Newnode);
                                    current = Newnode;
                                    flag = true;
                                    break;
                            }
                        }
                            if (flag)
                                break;
                    }
                        if (flag)
                            break;
                }
            }
            else {
                double obj = 0, vm, sigma_m;
                for (int j = 0; j < current->children.size(); j++) {
                    if (current->children[j]->score > obj) {
                        obj = current->children[j]->score;
                    }
                }
                double sum_um = 0;
                for (int j = 0; j < current->children.size(); j++) {
                    vm = current->children[j]->mv;
                    sigma_m = current->children[j]->sd;
                    if (sigma_m == 0) {
                        current->children[j]->um = 1;
                    }
                    else{
                        current->children[j]->um = erfc(10*(obj-vm)/(sqrt(2)*sigma_m));
                    }
                    sum_um += current->children[j]->um;
                   // current->children[j]->um = (score / current->children[j]->score) + sqrt(2)* sqrt(log(current->children[j]->num_times)/current->children[j]->num_times);
                }
                for (int j = 0; j < current->children.size(); j++) {
                    current->children[j]->pm = current->children[j]->um / sum_um;
                }
                double x = (double) rand() / (RAND_MAX + 1.0);
                double p1 = 0, p2 = 0;
               // double best = 0;
               // TreeNode *bestNode;
                for (int j = 0; j < current->children.size(); j++) {
                /*
                if ( current->children[j]->um > best ) {
                    best = current->children[j]->um;
                    bestNode = current->children[j];
                }
                */
                p2 += current->children[j]->pm;
                if (x >= p1 && x <= p2) {
                    current->children[j]->parent = current;
                    current = current->children[j];
                    break;
                }
                p1 = p2;
                }
               // bestNode->parent = current;
               // current = bestNode;
            }
        }
        if (!flag)
        times++;
    }
        while (current->readyTasks.size() != 0) {
            int min = 0, max = current->readyTasks.size()-1;
            int task_num = rand() % (max - min + 1) + min;
            min = 1;
            max = sysCPU;
            int sysCPU_num = rand() % (max - min + 1) + min;
            vector<Process> *scheduledTasks = new vector<Process>;
            vector<Process> *readyTasks = new vector<Process>;
            readyTasks = &current->readyTasks;
            scheduledTasks = &current->scheduledTasks;
            (*readyTasks)[task_num].request = sysCPU_num;
            scheduledTasks->push_back((*readyTasks)[task_num]);
            readyTasks->erase(readyTasks->begin()+task_num);
            TreeNode *Newnode = new TreeNode(current,*scheduledTasks, *readyTasks);
            current = Newnode;
        }
        double ATT = current->MLS(current);
        double inverse = 10000000000 - ATT;
        double score_sum, variance;
        if ( ATT < score ) {
            score = ATT;
            scheduledTasks.clear();
            scheduledTasks = current->scheduledTasks;
        }
        while (current->scheduledTasks.size() != 0) {
            score_sum = 0;
            current->num_times++;
            current->score_list.push_back(inverse);
            for (int q = 0; q < current->score_list.size(); q++) {
                score_sum += current->score_list[q];
            }
            current->mv = score_sum / current->score_list.size();
            variance = 0;
            for (int q = 0; q < current->score_list.size(); q++) {
                variance += pow(current->score_list[q] - current->mv, 2);
            }
            variance = variance / current->score_list.size();
            current->sd = sqrt(variance);
            if (current->score < inverse) {
                current->score = inverse;
            }
            current = current->parent;
        }
    }
    }
    else {
        for (int i = 1; i <= 1000; i++) {
        current = root;
        while (current->children.size() != 0) {
            double obj = 0, vm, sigma_m;
            for (int j = 0; j < current->children.size(); j++) {
                if (current->children[j]->score > obj) {
                    obj = current->children[j]->score;
                }
            }
            double sum_um = 0;
            for (int j = 0; j < current->children.size(); j++) {
                vm = current->children[j]->mv;
                sigma_m = current->children[j]->sd;
                if (sigma_m == 0) {
                        current->children[j]->um = 1;
                }
                else{
                    current->children[j]->um = erfc(10*(obj-vm)/(sqrt(2)*sigma_m));
                }
                sum_um += current->children[j]->um;
               // current->children[j]->um = (score / current->children[j]->score) + sqrt(2)* sqrt(log(current->children[j]->num_times)/current->children[j]->num_times);
            }
            for (int j = 0; j < current->children.size(); j++) {
                current->children[j]->pm = current->children[j]->um / sum_um;
            }
            double x = (double) rand() / (RAND_MAX + 1.0);
            double p1 = 0, p2 = 0;
            // double best = 0;
            // TreeNode *bestNode;
            for (int j = 0; j < current->children.size(); j++) {
                /*
                if ( current->children[j]->um > best ) {
                    best = current->children[j]->um;
                    bestNode = current->children[j];
                }
                */
                p2 += current->children[j]->pm;
                if (x >= p1 && x <= p2) {
                    current->children[j]->parent = current;
                    current = current->children[j];
                    break;
                }
                p1 = p2;
            }
            // bestNode->parent = current;
            // current = bestNode;
        }
        while (current->readyTasks.size() != 0) {
            int min = 0, max = current->readyTasks.size()-1;
            int task_num = rand() % (max - min + 1) + min;
            min = 1;
            max = sysCPU;
            int sysCPU_num = rand() % (max - min + 1) + min;
            vector<Process> *scheduledTasks = new vector<Process>;
            vector<Process> *readyTasks = new vector<Process>;
            readyTasks = &current->readyTasks;
            scheduledTasks = &current->scheduledTasks;
            (*readyTasks)[task_num].request = sysCPU_num;
            scheduledTasks->push_back((*readyTasks)[task_num]);
            readyTasks->erase(readyTasks->begin()+task_num);
            TreeNode *Newnode = new TreeNode(current,*scheduledTasks, *readyTasks);
            current = Newnode;
        }
        double ATT = current->MLS(current);
        double inverse = 10000000000 - ATT;
        double score_sum, variance;
        if ( ATT < score ) {
            score = ATT;
            scheduledTasks.clear();
            scheduledTasks = current->scheduledTasks;
        }
        while (current->scheduledTasks.size() != 0) {
            score_sum = 0;
            current->num_times++;
            current->score_list.push_back(inverse);
            for (int q = 0; q < current->score_list.size(); q++) {
                score_sum += current->score_list[q];
            }
            current->mv = score_sum / current->score_list.size();
            variance = 0;
            for (int q = 0; q < current->score_list.size(); q++) {
                variance += pow(current->score_list[q] - current->mv, 2);
            }
            variance = variance / current->score_list.size();
            current->sd = sqrt(variance);
            if (current->score < inverse) {
                current->score = inverse;
            }
            current = current->parent;
        }
    }
    }
    cout << "Global score: " << score << endl;
    leafNodes.clear();
    for (int i = 0; i < scheduledTasks.size(); i++) {
        cout << "Process " << scheduledTasks[i].procId << " -> " << i << "th" << "(" << scheduledTasks[i].request << ")" << "\t";
        scheduledTasks[i].priority = i;
        waitQueue.push(scheduledTasks[i]);
    }
    cout << endl;
    scheduledTasks.clear();
    score = 1000000;
}


void MCTS_online::schedule() {
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
        else {
            MCTS();
            break;
        }
    }
}
