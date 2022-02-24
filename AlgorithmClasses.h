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
    void MainLoop ();
    void PrintTaskGraph();
    void PrintAllPath();
    void FindAllPath(System* CurSystem);
    void SearchPath(std::shared_ptr<Task> CurTask, std::vector<std::shared_ptr<Task>> CurPath, System* CurSystem);
    
};


class MainAlgorithm
{
public:
public:
    MainAlgorithm(System* CurSystem);
    ~MainAlgorithm() = default;
    void MainLoop ();
};