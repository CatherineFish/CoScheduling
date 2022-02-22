#include "AlgorithmClasses.h"

TNode:: TNode(Task CurTask)
{
    CurrentTask = std::make_shared<Task>(CurTask.Period, CurTask.Time, CurTask.Left, CurTask.Right);
}

InitAlgorithm:: InitAlgorithm(System* CurSystem)
{
    int j = 0;
    for (const auto & CurTask: CurSystem->SystemTask)
    {
        TaskGraph.emplace_back(CurTask);
        for (size_t i = 0; i < CurTask.OutMessage.size(); i++)
        {
            TaskGraph[j].Children.push_back(std::make_shared<Task>(
                                            CurSystem->SystemTask[i].Period,
                                            CurSystem->SystemTask[i].Time,
                                            CurSystem->SystemTask[i].Left,
                                            CurSystem->SystemTask[i].Right));
        }
        j++;
    }

    j = 0;
    for (const auto & CurTask: CurSystem->SystemTask)
    {
        for (size_t i = 0; i < CurTask.OutMessage.size(); i++)
        {
            TaskGraph[i].Parents.push_back(std::make_shared<Task>(
                                            CurSystem->SystemTask[j].Period,
                                            CurSystem->SystemTask[j].Time,
                                            CurSystem->SystemTask[j].Left,
                                            CurSystem->SystemTask[j].Right));
        }
        j++;
    }
}

void InitAlgorithm:: PrintTaskGraph()
{
    std::cout << "======TaskGraph======" << std::endl;
    for (size_t i = 0; i < TaskGraph.size(); i++)
    {
        std::cout << "Task " << i << " :" << std::endl;
        std::cout << "Task Time: " << TaskGraph[i].CurrentTask->Time << std::endl;
        std::cout << "Task Children: " << std::endl;
        for (size_t j = 0; j < TaskGraph[i].Children.size(); j++)
        {
            std::cout << TaskGraph[i].Children[j]->Time << std::endl;
        }
        std::cout << "Task Parents: " << std::endl;
        for (size_t j = 0; j < TaskGraph[i].Parents.size(); j++)
        {
            std::cout << TaskGraph[i].Parents[j]->Time << std::endl;
        }
        std::cout << std::endl;
    }
    return;
}


void InitAlgorithm::GraphSearch(TNode CurNode, double CurTimeSum)
{

}

void InitAlgorithm:: MainLoop()
{
    for (size_t i = 0; i < TaskGraph.size(); i++)
    {
        if (TaskGraph[i].Parents.size() == 0)
        {
            GraphSearch(TaskGraph[i]);
        }
    }

}