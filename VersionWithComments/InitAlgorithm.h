#pragma once 

#include <vector>
#include <iostream>
#include <memory>

#include "MainClasses.h"
#include "CoefClasses.h"


class InitAlgorithm
{
public:
    std::vector<std::vector<std::shared_ptr<Task>>> AllPath;
    InitAlgorithm(System* CurSystem);
    ~InitAlgorithm() = default;
    void MainLoop (System* CurSystem);
    void PrintAllPath();
    void SearchPath(std::shared_ptr<Task> CurTask, std::vector<std::shared_ptr<Task>> & CurPath, System* CurSystem);
    
};
