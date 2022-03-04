#pragma once 

#include <vector>
#include <iostream>
#include <memory>

#include "MainClasses.h"
#include "CoefClasses.h"


class LimitedSearch
{

public:
    int SearchDepth;
    LimitedSearch(int Depth = 10): SearchDepth(Depth) {}
    ~LimitedSearch() = default;
    std::shared_ptr<PC> MainLoop(int Mode,
                                 std::shared_ptr<Job> CurJob,
                                 std::vector<std::shared_ptr<Job>> Planned,
                                 double PPoint);
    std::shared_ptr<PC> FirstSceme(std::shared_ptr<Job> CurJob,
                                   std::vector<std::shared_ptr<Job>> Planned,
                                   double PPoint,
                                   int CSearch = 1);
};