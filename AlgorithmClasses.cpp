#include "AlgorithmClasses.h"

TNode:: TNode(std::shared_ptr<Task> CurTask)
{
    CurrentTask = std::shared_ptr<Task>(CurTask);
}


InitAlgorithm:: InitAlgorithm(System* CurSystem)
{
    int j = 0;
    for (const auto & CurTask: CurSystem->SystemTask)
    {
        TaskGraph.emplace_back(CurTask);
        for (size_t i = 0; i < CurTask->OutMessage.size(); i++)
        {
            TaskGraph[j].Children.push_back(std::shared_ptr<Task>(CurSystem->SystemTask[CurTask->OutMessage[i]]));
        }
        j++;
    }

    j = 0;
    for (const auto & CurTask: CurSystem->SystemTask)
    {
        for (size_t i = 0; i < CurTask->OutMessage.size(); i++)
        {
            TaskGraph[CurTask->OutMessage[i]].Parents.push_back(std::shared_ptr<Task>(CurSystem->SystemTask[j]));
        }
        j++;
    }
}

void InitAlgorithm:: PrintTaskGraph()
{
    std::cout << "======TaskGraph======" << std::endl;
    for (size_t i = 0; i < TaskGraph.size(); i++)
    {
        std::cout << "Task " << i << " :" << std::endl;
        std::cout << "Task Time: " << TaskGraph[i].CurrentTask->Time << std::endl;
        std::cout << "Task Children: " << std::endl;
        for (size_t j = 0; j < TaskGraph[i].Children.size(); j++)
        {
            std::cout << TaskGraph[i].Children[j]->Time << std::endl;
        }
        std::cout << "Task Parents: " << std::endl;
        for (size_t j = 0; j < TaskGraph[i].Parents.size(); j++)
        {
            std::cout << TaskGraph[i].Parents[j]->Time << std::endl;
        }
        std::cout << std::endl;
    }
    return;
}



void InitAlgorithm:: SearchPath(std::shared_ptr<Task> CurTask, std::vector<std::shared_ptr<Task>> CurPath, System* CurSystem)
{
    /*
    std::cout << "-------NEW TASK-------" << std::endl;
    std::cout << CurTask.Time << " " << CurTask.OutMessage.size() << std::endl;
    std::cout << "CURRENT VECTOR" << std::endl;
    for (size_t i = 0; i < CurPath.size(); i++)
    {
        std::cout << CurPath[i]->Time << " ";
    }
    std::cout << std::endl;
    */
    if (CurTask->OutMessage.size() == 0)
    {
        CurPath.push_back(std::shared_ptr<Task>(CurTask));
        AllPath.push_back(CurPath);
        CurPath.pop_back();
        return;    
    }

    CurPath.push_back(std::shared_ptr<Task>(CurTask));
    for (size_t i = 0; i < CurTask->OutMessage.size(); i++)
    {
        SearchPath(CurSystem->SystemTask[CurTask->OutMessage[i]], CurPath, CurSystem);
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

void InitAlgorithm:: FindAllPath(System* CurSystem)
{
    double CurTimeSum, CurMesTime, NewDur, SumMesSize;
    int Mode, MessageCount;
    std::cout << "Как распределить пропускную способность?" << std::endl;
    std::cout << "1 - Поровну" << std::endl;
    std::cout << "2 - Пропорционально" << std::endl;
    std::cin >> Mode;  
    for (size_t i = 0; i < CurSystem->SystemTask.size(); i++)
    {
        //std::cout << i << std::endl;
        if (CurSystem->SystemTask[i]->InMessage.size() == 0)
        {
            std::vector<std::shared_ptr<Task>> NewPath;
            //std::cout << "ROOT VERTEX NUM: " << i << std::endl;
            SearchPath(CurSystem->SystemTask[i], NewPath, CurSystem);   
        }
    }
    //PrintAllPath();  
    
    PrintAllPath();  
    for (size_t i = 0; i < AllPath.size(); i++)
    {
        std::cout << "PATH " << i << std::endl;
        CurTimeSum = 0.0;
        SumMesSize = 0.0;
        for (size_t j = 0; j < AllPath[i].size(); j++)
        {
            CurTimeSum += AllPath[i][j]->Time;
            for (const auto & CurMes: AllPath[i][j]->MesOut)
            {
                if (CurMes->Dest == AllPath[i][j + 1])
                {
                    SumMesSize += CurMes->Size;
                }
            }
        }

        MessageCount = AllPath[i].size() - 1;
        CurMesTime = AllPath[i][AllPath[i].size() - 1]->Right - CurTimeSum;
        std::cout << "SUM TIME FOR EXECUTION = " << CurTimeSum << std::endl;
        std::cout << "RIGHT TIME = " << AllPath[i][AllPath[i].size() - 1]->Right << std::endl;
        std::cout << "MESSAGE COUNT = " << MessageCount << std::endl;
        std::cout << "TIME FOR MESSAGES = " << CurMesTime << std::endl;
        if (Mode == 1)
        {
            NewDur = CurMesTime / MessageCount;
            std::cout << "NEW DUR = " << NewDur << std::endl;
            for (size_t j = 0; j < AllPath[i].size() - 1; j++)
            {
                for (const auto & CurMes: AllPath[i][j]->MesOut)
                {
                    std::cout << CurMes->Dest->Time << " " << AllPath[i][j + 1]->Time << std::endl;
                    if (CurMes->Dest == AllPath[i][j + 1])
                    {
                        CurMes->Dur = std::max(CurMes->Dur, NewDur);
                    }
                }
            }   
        } else if (Mode == 2)
        {
            for (size_t j = 0; j < AllPath[i].size() - 1; j++)
            {
                for (const auto & CurMes: AllPath[i][j]->MesOut)
                {
                    if (CurMes->Dest == AllPath[i][j + 1])
                    {
                        CurMes->Dur = std::max(CurMes->Dur, (CurMesTime * CurMes->Size) / SumMesSize);
                    }
                }
            }   
        } else
        {
            std::cout << "Invalid Mode" << std::endl;
            exit(0);
        }
        for (size_t j = 0; j < AllPath[i].size() - 1; j++)
        {
            for (const auto & CurMes: AllPath[i][j]->MesOut)
            {
                if (CurMes->Dest == AllPath[i][j + 1])
                {
                    CurMes->Bandwidth = CurMes->Size / CurMes->Dur;
                }
            }
        }   

    }
    //PrintAllPath();  
    
    CurSystem->PrintSystem();
    return;
}