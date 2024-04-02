#include <iostream>
#include <vector>
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

list<Task> s_list;
list<Task> runTable;
list<Task>::iterator it;
list<Task>::iterator s;
int totalCPUNum = 32;

struct cmp {
    bool operator ()(Task a, Task b) {
        return (a.arrivalTime == b.arrivalTime) ? (a.status < b.status) :
        (a.arrivalTime < b.arrivalTime);
    };
};

struct work {
    bool operator ()(Task a, Task b) {
        return (a.execTime*a.cpu == b.execTime*b.cpu) ? (a.procID < b.procID) :
        (a.execTime*a.cpu < b.execTime*b.cpu);
    };
};

int schedule() {
    bool isCPUIdle;
    int usage = totalCPUNum, anchor_point, maximum = 0, turnAroundTime = 0;
    for (s = s_list.begin(); s != s_list.end(); s++) {
        anchor_point = 0;
        isCPUIdle = true;
        for (it = runTable.begin(); it != runTable.end(); it++) {
            if ( (usage >= (*s).cpu) && ((*it).getArrivalTime() > anchor_point) ) {
                isCPUIdle = false;
            }
            if ( (*it).status == Task::READY ) {
                if (isCPUIdle) {
                cout << "Running process " << (*it).procID << " Released resources " << (*it).cpu << " Terminated time " << (*it).getArrivalTime() << endl;
                anchor_point = (*it).getArrivalTime();
                usage += (*it).cpu;
                    cout << "Usage: " << usage << endl;
                }
            }
            else if ( (*it).status == Task::ESTIMATE ) {
                if ( (*it).remain_node < (*s).cpu ) {
                    usage = (*it).remain_node;
                    isCPUIdle = true;
                }
                else if ( (*it).remain_node >= (*s).cpu ) {
                    usage = (*it).remain_node;
                    (*it).remain_node -= (*s).cpu;
                }
            }
        }
       // int t = (*s).execTime*(0.3 + 0.7/(*s).cpu);
        int t = (*s).execTime;
        
        Task *run = new Task((*s).procID, anchor_point + t, t, (*s).cpu);
        run->status = Task::READY;
        runTable.push_back(*run);
        usage -= (*s).cpu;
        (*s).finishTime = anchor_point + t;
        if ((*s).finishTime > maximum) {
            maximum = (*s).finishTime;
        }
        turnAroundTime += (*s).finishTime;
        
        Task *Proc = new Task((*s).procID, anchor_point, t, (*s).cpu);
        Proc->status = Task::ESTIMATE;
        Proc->remain_node = usage;
        Proc->finishTime = anchor_point + t;
        runTable.push_back(*Proc);
        (*s).StartTime = anchor_point;
        
        cout << "Process " << (*s).procID << ":" << endl <<
        "Resource need: " << (*s).cpu << endl <<
        "Remain node: " << usage << endl <<
        "Submit time: " << (*s).getArrivalTime() << endl <<
        "Anchor point: " << anchor_point << endl <<
        "Terminated time: " << anchor_point + t << endl << endl;
        runTable.sort(cmp());
    }
    cout << "Average turnaround time: " << turnAroundTime / s_list.size() << endl;
    runTable.clear();
    s_list.clear();
    return maximum;
}


int main() {
    ifstream read("l_sdsc_sp2.swf.extracted", ios::in);
    string ch;
    int load_factor = 1;
    int count;
    int jobCount = 0;
    int test = 0, record = 0;
    int n[17];
    int makespan = 0;

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
        if ( record >= 1 ) {
            if ( n[10] == 1 ) {
            test++;
            n[1] = 0;
            Task *Proc = new Task(jobCount, n[1], n[3], n[7]);
            Proc->status = Task::READY;
            s_list.push_back(*Proc);
            jobCount++;
            }
        }
        if ( test == 10 )
            break;
    }
    s_list.sort(work());
    makespan = schedule();
    cout << "Makespan: " << makespan << endl;
    
    return 0;
}
