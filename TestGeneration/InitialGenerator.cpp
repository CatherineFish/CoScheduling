
#include <cmath>
#include <vector>
#include <random>
#include <map>
#include <iostream>
#include <numeric>

#include "InitialGenerator.h"
#include "../MainClasses.h"


InitialGenerator :: InitialGenerator(int LCM_, int TaskNum_, int PCNum_, int ModNum_):
    TaskNum(TaskNum_), PCNum(PCNum_), ModNum(ModNum_)
{
    LCM = int(std::pow(2, LCM_));
    std::vector<int> NumOfPeriodTask;

    std::random_device rd{};
    std::mt19937 gen{rd()};
 
    std::normal_distribution<double> d{LCM_ / 2.0, LCM_ / 4.0};
    //TODO какая должна быть дисперсия?
 
    std::map<int, int> distribute{};
    for(int n = 0; n < TaskNum;) {
        int number = std::round(d(gen));
        if ((number >= 1) && (number <= LCM_)) {
            ++distribute[number];
            n++;
        }
    }
    std::cout << "ALL: " << std::endl;
    for(auto p : distribute) {
        std::cout << p.first << ' ' << std::string(p.second, '*') << '\n';
    }

    std::vector<std::map<int, int>> PCandTask;
    for (int i = 0; i < PCNum; i++)
    {
        PCandTask.push_back(std::map<int, int>{});
    }
    size_t k = 0;
    for(auto curTask : distribute) {
        int ForAll = curTask.second / PCNum;
        for (size_t i = 0; i < PCandTask.size(); i++)
        {
            PCandTask[i][curTask.first] = ForAll;
        }
        for (size_t i = k; i < curTask.second - ForAll * PCandTask.size(); i++)
        {
            PCandTask[i][curTask.first]++;
            k++;
            if (k >= PCandTask.size())
                k = 0;
        }
    }

    std::cout << "CP: " << std::endl;
    
    for (int i = 0; i < PCNum; i++)
    {
        for(auto p : PCandTask[i]) {
            std::cout << p.first << ' ' << std::string(p.second, '*') << '\n';
        }
        std::cout << std::endl;
    
    }

    for (size_t i = 0; i < PCandTask.size(); i++)
    {
        GeneratorForPC(PCandTask[i], LCM_);
    }

    //GeneratorForPC(PCandTask[0], LCM_);
    //GeneratorForPC(PCandTask[1], LCM_);
    for (size_t i = 0; i < AllTasks.size(); i++)
    {
        AllTasks[i]->Time = AllTasks[i]->Right - AllTasks[i]->Left;
        AllTasks[i]->Left = 0;
        AllTasks[i]->Right = AllTasks[i]->Period;
        std::cout << "Task " << i << " : " << std::endl;
        std::cout << "Left : " << AllTasks[i]->Left << std::endl;
        std::cout << "Right : " << AllTasks[i]->Right << std::endl;
        std::cout << "Time : " << AllTasks[i]->Time << std::endl;
        std::cout << "----------------------------------" << std::endl;
           
    }
    
}


void InitialGenerator :: GeneratorForPC(std::map<int, int> TaskMap, int LCM_)
{
    std::vector<Job> AllJobs;
    int NumOfPeriod = LCM / std::pow(2, (*TaskMap.begin()).first);
    std::vector<std::vector<int>> AllPeriods;
    std::vector<std::vector<std::shared_ptr<Task>>> PtrAllPeriods;
    std::vector<int> k (LCM_);
    std::iota (std::begin(k), std::end(k), 0);
    for (int i = 0; i < NumOfPeriod; i++)
    {
        AllPeriods.push_back(std::vector<int>{});
        PtrAllPeriods.push_back(std::vector<std::shared_ptr<Task>>{});
    }
    for (auto curTask : TaskMap)
    {
        
        for (int j = 0; j < curTask.second; j++)
        {
            AllTasks.push_back(std::make_shared<Task>(std::pow(2, curTask.first)));
        
            for (int i = k[curTask.first - 1]; i < NumOfPeriod; i = i + curTask.first)
            {

                AllPeriods[i].push_back(curTask.first);
                PtrAllPeriods[i].push_back(std::shared_ptr<Task>(AllTasks[AllTasks.size() - 1]));
                //std::cout << PtrAllPeriods[i].size() << " ";
                
            }
            k[curTask.first - 1]++;
            if (k[curTask.first - 1] > curTask.first - 1)
                k[curTask.first - 1] = 0;
        }
    }
    for (size_t i = 0; i < PtrAllPeriods.size(); i++)
    {
        std::cout << "Period : " << i + 1 << std::endl;
        for (size_t j = 0; j < PtrAllPeriods[i].size(); j++)
        {
            std::cout << PtrAllPeriods[i][j]->Period << " ";
        }
        std::cout << std::endl;
    }

    /*std::cout << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < AllPeriods.size(); i++)
    {
        std::cout << "Period : " << i + 1 << std::endl;
        
        for (size_t j = 0; j < AllPeriods[i].size(); j++)
        {
            std::cout << AllPeriods[i][j] << " ";
        }
        std::cout << std::endl;
    }*/

    int MaxLenght = 0, Idx = 0;
    for (size_t i = 0; i < PtrAllPeriods.size(); i++)
    {
        if (MaxLenght < PtrAllPeriods[i].size())
        {
            MaxLenght = PtrAllPeriods[i].size();
            Idx = i;
        }
    }
    double Dur = std::pow(2, (*TaskMap.begin()).first) / MaxLenght;
    for (size_t i = 0; i < PtrAllPeriods.size(); i++)
    {
        for (size_t j = 0; j < PtrAllPeriods[i].size(); j++)
        {
            if (PtrAllPeriods[i][j]->Right != 0) continue;
            PtrAllPeriods[i][j]->Left = j * Dur + i * std::pow(2, (*TaskMap.begin()).first) - 
                                     int(i * std::pow(2, (*TaskMap.begin()).first) / PtrAllPeriods[i][j]->Period) * PtrAllPeriods[i][j]->Period; 
            PtrAllPeriods[i][j]->Right = PtrAllPeriods[i][j]->Left + Dur;   
               
        }
        
    }
    /*for (int i = 0; i < MaxLenght; i++)
    {
        PtrAllPeriods[Idx][i]->Left = i * Dur + Idx * std::pow(2, (*TaskMap.begin()).first) - 
                                     int(Idx * std::pow(2, (*TaskMap.begin()).first) / PtrAllPeriods[Idx][i]->Period) * PtrAllPeriods[Idx][i]->Period; 
        PtrAllPeriods[Idx][i]->Right = PtrAllPeriods[Idx][i]->Left + Dur;   
        for (size_t j = 0; j < AllTasks.size(); j++)
        {
            std::cout << "(" << AllTasks[j]->Left << " " << AllTasks[j]->Right << " per : " << AllTasks[j]->Period << ") - " << j << "    ";
        }
        std::cout << std::endl;
        
    }*/

    for (size_t i = 0; i < PtrAllPeriods.size(); i++)
    {
        std::cout << "Period : " << i + 1 << std::endl;
        for (size_t j = 0; j < PtrAllPeriods[i].size(); j++)
        {
            std::cout << "(" << PtrAllPeriods[i][j]->Left << ", " << PtrAllPeriods[i][j]->Right << ")  ";
        }
        std::cout << std::endl;
    }
    return;
}
