#include <algorithm>
#include <limits>
#include "InitAlgorithm.h"


void InitAlgorithm:: SearchPath(std::shared_ptr<Task> CurTask, std::vector<std::shared_ptr<Task>> CurPath, System* CurSystem)
{
    CurPath.push_back(std::shared_ptr<Task>(CurTask));
    
    if (CurTask->OutMessage.size() == 0)
    {
        AllPath.push_back(CurPath);
    } else 
    {
        for (size_t i = 0; i < CurTask->OutMessage.size(); i++)
        {
            SearchPath(CurSystem->SystemTask[CurTask->OutMessage[i]], CurPath, CurSystem);
        }
    }
    CurPath.pop_back();
    return;
}

void InitAlgorithm:: PrintAllPath()
{
    std::cout << "======All Graph Path======" << std::endl;
    for (size_t i = 0; i < AllPath.size(); i++)
    {
        std::cout << "---- Path " << i << " ----" << std::endl;
        for (size_t j = 0; j < AllPath[i].size(); j++)
        {
            std::cout << AllPath[i][j]->Time << " " << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << "=========================" << std::endl;
    
}

InitAlgorithm::InitAlgorithm(System* CurSystem)
{
    for (size_t i = 0; i < CurSystem->SystemTask.size(); i++)
    {
        if (CurSystem->SystemTask[i]->InMessage.size() == 0)
        {
            std::vector<std::shared_ptr<Task>> NewPath;
            SearchPath(CurSystem->SystemTask[i], NewPath, CurSystem);   
        }
    }
}

void InitAlgorithm:: MainLoop(System* CurSystem)
{
    double minLeft, maxLeft;
    double SumBand = 0.0;
    PrintAllPath();  
    for (size_t i = 0; i < AllPath.size(); i++)
    {
        std::cout << "Path " << i << std::endl;
        minLeft = 0.0;
        for (size_t j = 0; j < AllPath[i].size(); j++)
        {
            AllPath[i][j]->MinLeft = std::max(AllPath[i][j]->MinLeft, minLeft);
            minLeft = AllPath[i][j]->MinLeft + AllPath[i][j]->Time;
        }
        maxLeft = CurSystem->LCMPeriod;
        for (int j = AllPath[i].size() - 1; j >= 0; j--)
        {
            maxLeft -= AllPath[i][j]->Time;
            AllPath[i][j]->MaxLeft = std::min(AllPath[i][j]->Right - AllPath[i][j]->Time, maxLeft);
            maxLeft = AllPath[i][j]->MaxLeft;
        }    
    }

    for (size_t i = 0; i < AllPath.size(); i++)
    {
        std::cout << "Path " << i << std::endl;
        
        for (size_t j = 0; j < AllPath[i].size(); j++)
        {
            if (AllPath[i][j]->IsInit)
            {
                continue;
            }
            std::cout << "Task : " << j << " Add: " << (AllPath[i][j]->MaxLeft - AllPath[i][j]->MinLeft) / 2 << " To " << AllPath[i][j]->Left << " Right " << AllPath[i][j]->Right << " Time " << AllPath[i][j]->Time << std::endl; 
            if (j == 0)
            {
                AllPath[i][j]->Left = AllPath[i][j]->MinLeft;
            } else 
            {
                AllPath[i][j]->Left = AllPath[i][j]->MinLeft + (AllPath[i][j]->MaxLeft - AllPath[i][j]->MinLeft) / 2;
            }
            if (AllPath[i][j]->Left + AllPath[i][j]->Time > AllPath[i][j]->Right) {
                std::cout << "PROBLEM: input data are incorrect" << std::endl;
                exit(1);
            }
            AllPath[i][j]->Right = AllPath[i][j]->Left + AllPath[i][j]->Time;
            
            
            AllPath[i][j]->IsInit = true;
        }
    }
    for (const auto & CurMes : CurSystem->SystemMessage)
    {
        CurMes->Dur = CurMes->Dest->Left - CurMes->Src->Right;
        if (CurMes->Dur == 0)
        {
            CurMes->Bandwidth = std::numeric_limits<double>::max();
        } else 
        {
            CurMes->Bandwidth = CurMes->Size / CurMes->Dur;   
            SumBand += CurMes->Bandwidth;
        }            
    }
    CurSystem->CurBLackCoef = BLackCoef(SumBand, CurSystem->BTotal); 
    
    return;
}