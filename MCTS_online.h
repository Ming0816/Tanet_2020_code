#include "Scheduler.h"
using namespace std;

class MCTS_online : public Scheduler<ProcessQueue> {

    public:
    MCTS_online(EventPriorityQueue *eventQueue);
        virtual ~MCTS_online();
        void schedule();
        void MCTS();
};

class TreeNode {
public:

    double num_times;
    double mv;
    double sd;
    double score;
    double um;
    double pm;
    TreeNode* parent;
    vector<TreeNode*> children;
    vector<Process> readyTasks;
    vector<Process> scheduledTasks;
    vector<double> score_list;
    TreeNode(TreeNode* parent, vector<Process> x, vector<Process> y);
    double MLS(TreeNode* node);
    double Action_selection(TreeNode* node);
};