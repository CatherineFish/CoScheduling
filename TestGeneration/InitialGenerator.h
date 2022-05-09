#pragma once 

#include <map>
#include <vector>

#include "../MainClasses.h"


class InitialGenerator
{
public:
    int LCM;
    int TaskNum, PCNum, ModNum, MesNum;
    InitialGenerator(char * Filename, int LCM_, int TaskNum_, int PCNum_, int ModNum_, int MesNum_, int Mean, int Disp, int MaxSize);
    ~InitialGenerator() = default;
    void GeneratorForPC(std::map<int, int> TaskMap, int LCM_);
    void GeneratorForPC2(std::map<int, int> TaskMap, int LCM_);
    
    std::vector<std::shared_ptr<Task>> AllTasks {};
    std::vector<std::shared_ptr<Job>> AllJobs {};
    std::map<std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>, std::shared_ptr<Message>> JobMessage;
    
    std::vector<std::shared_ptr<Message>> Mes {};
    void GeneratorForMessage(int Mean, int Disp, int MaxSize);
        
    std::vector<std::vector<std::vector<int>>> AllPeriods;
    std::vector<std::vector<std::vector<std::shared_ptr<Task>>>> PtrAllPeriods;
    
    std::vector<std::vector<int>> ModVec;
    double Band = 0.0;

    void GenerateInputFile (char *filename, 
                        std::vector<int> ModNum);
    int MaxLenght = 0;
    void ResultCheck();
    
};
