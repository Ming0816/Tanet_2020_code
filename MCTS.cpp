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
#include <cmath>
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
int totalCPUNum = 32;

struct cmp {
    bool operator ()(Task a, Task b) {
        return (a.arrivalTime == b.arrivalTime) ? (a.status < b.status) :
        (a.arrivalTime < b.arrivalTime);
    };
};
 
class TreeNode {
public:

    double mv;
    double sd;
    double score;
    double um;
    double pm;
    double num_times;
    TreeNode* parent;
    vector<TreeNode*> children;
    vector<Task> readyTasks;
    vector<Task> scheduledTasks;
    vector<double> score_list;
    TreeNode(TreeNode* parent, vector<Task> x, vector<Task> y);
    double MLS(TreeNode* node);
    double Action_selection(TreeNode* node);
};


TreeNode::TreeNode(TreeNode* parent, vector<Task> x, vector<Task> y) : parent(parent) {
    this->num_times = 1;
    this->mv = 0;
    this->sd = 0;
    this->score = 1;
    this->um = 0;
    this->pm = 0;
    this->scheduledTasks = x;
    this->readyTasks = y;
}

double TreeNode::MLS(TreeNode* node) {
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
                anchor_point = (*it).getArrivalTime();
                usage += (*it).cpu;
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
        t = node->scheduledTasks[i].execTime*(0.3 + (0.7/node->scheduledTasks[i].cpu));
        Task *run = new Task(node->scheduledTasks[i].procID, anchor_point + t, t, node->scheduledTasks[i].cpu);
        run->status = Task::READY;
        runTable.push_back(*run);
        usage -= node->scheduledTasks[i].cpu;
        node->scheduledTasks[i].finishTime = anchor_point + t;
        maximum += node->scheduledTasks[i].finishTime;
        sum += node->scheduledTasks[i].arrivalTime;
        
        Task *Proc = new Task(node->scheduledTasks[i].procID, anchor_point, t, node->scheduledTasks[i].cpu);
        Proc->status = Task::ESTIMATE;
        Proc->remain_node = usage;
        Proc->finishTime = anchor_point + t;
        runTable.push_back(*Proc);
        node->scheduledTasks[i].StartTime = anchor_point;
        runTable.sort(cmp());
    }
    runTable.clear();
    return (maximum - sum) / node->scheduledTasks.size();
}


int main() {
    ifstream read("l_sdsc_sp2.swf.extracted", ios::in);
    string ch;
   // double score = std::numeric_limits<double>::max();
    double score = 1000000;
    int length;
    int load_factor = 1;
    int count;
    int jobCount = 0;
    int test = 0, record = 0;
    int n[17];
    vector<Task> scheduledTasks;
    vector<Task> *alltasks = new vector<Task>;
    vector<TreeNode*> leafNodes;
    vector<TreeNode*> temp_list;
    TreeNode *head = NULL;
    vector<Task> *scheduled = new vector<Task>;
    vector<Task> *ready = new vector<Task>;
    TreeNode *root = new TreeNode(head, *scheduled, *ready);
    TreeNode *current = root;

    while ( getline(read,ch) ) {
        record++;
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
        if ( record >= 200 ) {
            if ( n[10] == 1 ) {
            test++;
            n[1] = 0;
            n[3] = (double)n[3]*load_factor/(0.3+(0.7/n[7]));
            Task *Proc = new Task(jobCount, n[1], n[3], n[7]);
            Proc->status = Task::READY;
            alltasks->push_back(*Proc);
            cout << "Job " << jobCount << " Serial time: " << n[3] << endl;
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
        TreeNode *Newnode = new TreeNode(root,*v1, *v2);
        root->children.push_back(Newnode);
        leafNodes.push_back(Newnode);
        }
    }
    /*
    for (int i = 0; i < 2; i++) {
        length = leafNodes.size();
        for (int j = 0; j < length; j++) {
            current = leafNodes[0];
            leafNodes.erase(leafNodes.begin());
            for (int m = 0; m < current->readyTasks.size(); m++) {
                vector<Task> *scheduledTasks = new vector<Task>;
                vector<Task> *readyTasks = new vector<Task>;
                readyTasks = &current->readyTasks;
                scheduledTasks = &current->scheduledTasks;
                scheduledTasks->push_back((*readyTasks)[m]);
                readyTasks->erase(readyTasks->begin()+m);
                for (int p = 1; p <= totalCPUNum; p++) {
                vector<Task> *v1 = new vector<Task>;
                vector<Task> *v2 = new vector<Task>;
                v1 = scheduledTasks;
                v2 = readyTasks;
                (*v1)[v1->size()-1].cpu = p;
                TreeNode *Newnode = new TreeNode(current,*v1, *v2);
                temp_list.push_back(Newnode);
                current->children.push_back(Newnode);
                }
            }
        }
        leafNodes = temp_list;
        temp_list.clear();
    }
    */
    srand(1);
    for (int i = 1; i <= 1000; i++) {
        current = root;
        while (current->children.size() != 0) {
            double obj = 0, vm, sigma_m;
            /*
            for (int j = 0; j < current->children.size(); j++) {
                if (current->children[j]->score > obj) {
                    obj = current->children[j]->score;
                }
            }
            */
           // double sum_um = 0;
            for (int j = 0; j < current->children.size(); j++) {
                vm = current->children[j]->mv;
                sigma_m = current->children[j]->sd;
               // cout << "number of times: " << current->children[j]->num_times << endl;
                current->children[j]->um = (score / current->children[j]->score) + sqrt(2)* sqrt(log(current->children[j]->num_times)/current->children[j]->num_times);
                /*
                current->children[j]->um = erfc(10*(obj-vm)/(sqrt(2)*sigma_m));
                if (sigma_m == 0) {
                    current->children[j]->um = 1;
                }
                sum_um += current->children[j]->um;
                */
            }
            /*
            for (int j = 0; j < current->children.size(); j++) {
                current->children[j]->pm = current->children[j]->um / sum_um;
            }
            double x = (double) rand() / (RAND_MAX + 1.0);
            double p1 = 0, p2 = 0;
             */
            double best = 0;
            TreeNode *bestNode;
            for (int j = 0; j < current->children.size(); j++) {
               // p2 += current->children[j]->pm;
               // if (x >= p1 && x <= p2) {
                if ( current->children[j]->um > best ) {
                    best = current->children[j]->um;
                    bestNode = current->children[j];
                }
                   // break;
               // }
               // p1 = p2;
            }
            bestNode->parent = current;
            current = bestNode;
        }
        while (current->readyTasks.size() != 0) {
            int min = 0, max = current->readyTasks.size()-1;
            int task_num = rand() % (max - min + 1) + min;
            min = 1;
            max = totalCPUNum;
            int sysCPU_num = rand() % (max - min + 1) + min;
            vector<Task> *scheduledTasks = new vector<Task>;
            vector<Task> *readyTasks = new vector<Task>;
            readyTasks = &current->readyTasks;
            scheduledTasks = &current->scheduledTasks;
            (*readyTasks)[task_num].cpu = sysCPU_num;
            scheduledTasks->push_back((*readyTasks)[task_num]);
            readyTasks->erase(readyTasks->begin()+task_num);
            TreeNode *Newnode = new TreeNode(current,*scheduledTasks, *readyTasks);
            current = Newnode;
        }
        double ATT = current->MLS(current);
        double inverse = 100000 - ATT;
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
    cout << "Global score: " << score << endl << endl;
    scheduledTasks.clear();
    leafNodes.clear();
    return 0;
}
