
#pragma once 

#include <map>
#include <vector>

#include "../MainClasses.h"


class InitialGenerator
{
public:
    int LCM;
    int TaskNum, PCNum, ModNum;
    InitialGenerator(int LCM_, int TaskNum_, int PCNum_, int ModNum_);
    ~InitialGenerator() = default;
    void GeneratorForPC(std::map<int, int> TaskMap, int LCM_);
    std::vector<std::shared_ptr<Task>> AllTasks;
    


};
