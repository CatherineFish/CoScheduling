#pragma once 

#include <vector>
#include <iostream>
#include <memory>

#include "MainClasses.h"
#include "CoefClasses.h"


class MainAlgorithm
{
public:
    std::vector<std::shared_ptr<Job>> Unplanned;
    std::vector<std::shared_ptr<Job>> Planned;   
    MainAlgorithm(System* CurSystem, int Mood_ = 1);
    ~MainAlgorithm() = default;
    void MainLoop (System* CurSystem, int Depth, int Search);
    void UpdateBList(System* CurSystem);
    void UpdateFList(System* CurSystem);
    void UpdateRList();
    double UpdatePPoint(System* CurSystem);
    static bool SortBySlack(std::shared_ptr<Job> i, std::shared_ptr<Job> j);
    double CritLimit; // Experiment
    double LimitForPC; // Experiment
    void PrintJobSystem(System* CurSystem);
    void PrintJobSystemWithoutLists();
    int Check(std::shared_ptr<Job> CurJob, std::shared_ptr<PC> PCForPlan, System* CurSystem);
    void ResultCheck(System * CurSystem);
    int Mood; // Experiment
};