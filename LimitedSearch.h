#pragma once 

#include <vector>
#include <iostream>
#include <memory>

#include "MainClasses.h"
#include "CoefClasses.h"


class LimitedSearch
{

public:
    int SearchDepth = 5; //TODO
    int IterationDepth = 2; //TODO
    LimitedSearch(int Depth = 10): SearchDepth(Depth) {}
    ~LimitedSearch() = default;
    std::shared_ptr<PC> MainLoop(int Mode,
                                 std::shared_ptr<Job> CurJob,
                                 std::vector<std::shared_ptr<Job>> Planned,
                                 System * CurSystem);
    std::shared_ptr<PC> FirstSceme(std::shared_ptr<Job> CurJob,
                                   std::shared_ptr<PC> BadPC, 
                                   std::vector<std::shared_ptr<Job>> Planned,
                                   System* CurSystem,
                                   int CSearch = 1,
                                   int Iteration = 1);
    int Check(std::shared_ptr<Job> CurJob,
              std::shared_ptr<PC> PCForPlan,
              System* CurSystem,
              double NewPPoint);
};