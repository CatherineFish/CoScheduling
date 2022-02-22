#pragma once 
#include <vector>
#include <iostream>
#include <memory>

#include "MainClasses.h"

class TNode
{
public:
    std::shared_ptr<Task> CurrentTask;
    std::vector<std::shared_ptr<Task>> Parents;
    std::vector<std::shared_ptr<Task>> Children;
    TNode(Task CurTask);
    ~TNode() = default;
};

class InitAlgorithm
{
public:
    std::vector<TNode> TaskGraph;
public:
    InitAlgorithm(System* CurSystem);
    ~InitAlgorithm() = default;
    void MainLoop ();
    void PrintTaskGraph();

};