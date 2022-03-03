#pragma once 

#include <vector>
#include <iostream>
#include <memory>

#include "MainClasses.h"
#include "CoefClasses.h"

class TNode
{
public:
    std::shared_ptr<Task> CurrentTask;
    std::vector<std::shared_ptr<Task>> Parents;
    std::vector<std::shared_ptr<Task>> Children;
    TNode(std::shared_ptr<Task> CurTask);
    ~TNode() = default;
};



class InitAlgorithm
{
public:
    std::vector<TNode> TaskGraph;
    std::vector<std::vector<std::shared_ptr<Task>>> AllPath;
public:
    InitAlgorithm(System* CurSystem);
    ~InitAlgorithm() = default;
    void MainLoop (System* CurSystem);
    //void PrintTaskGraph();
    void PrintAllPath();
    //void FindAllPath(System* CurSystem);
    void SearchPath(std::shared_ptr<Task> CurTask, std::vector<std::shared_ptr<Task>> CurPath, System* CurSystem);
    
};


class MainAlgorithm
{
public:
public:
    std::vector<std::shared_ptr<Job>> Unplanned;
    std::vector<std::shared_ptr<Job>> Planned;
    
    
    MainAlgorithm(System* CurSystem);
    ~MainAlgorithm() = default;
    void MainLoop (System* CurSystem);
    void UpdateBList(System* CurSystem);
    void UpdateFList(std::vector<std::shared_ptr<PC>> SystemPC);
    void UpdateRList();
    double UpdatePPoint();
    
    static bool SortBySlack(std::shared_ptr<Job> i, std::shared_ptr<Job> j);
    double CritLimit = 100000.0; //TODO
    void PrintJobSystem(System* CurSystem);
    void UpdateLeft(std::shared_ptr<Job> CurJob, System* CurSystem);
    int Check(std::shared_ptr<Job> CurJob, std::shared_ptr<PC> PCForPlan, System* CurSystem);

};