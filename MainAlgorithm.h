#pragma once 

#include <vector>
#include <iostream>
#include <memory>

#include "MainClasses.h"
#include "CoefClasses.h"


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
    double UpdatePPoint(System* CurSystem);
    
    static bool SortBySlack(std::shared_ptr<Job> i, std::shared_ptr<Job> j);
    double CritLimit = 150.0; //TODO
    void PrintJobSystem(System* CurSystem);
    void UpdateLeft(std::shared_ptr<Job> CurJob, System* CurSystem);
    int Check(std::shared_ptr<Job> CurJob, std::shared_ptr<PC> PCForPlan, System* CurSystem);

};