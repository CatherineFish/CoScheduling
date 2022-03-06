#include <algorithm>

#include "InitAlgorithm.h"

/*TNode:: TNode(std::shared_ptr<Task> CurTask)
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

*/

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
    double CurTimeSum, CurMesTime, NewDur, SumMesSize;
    int Mode, MessageCount;
    double CurBandwidth = 0.0;
    std::cout << "Как распределить пропускную способность?" << std::endl;
    std::cout << "1 - Поровну" << std::endl;
    std::cout << "2 - Пропорционально" << std::endl;
    std::cin >> Mode;  
    PrintAllPath();  
    for (size_t i = 0; i < AllPath.size(); i++)
    {
        CurTimeSum = 0.0;
        SumMesSize = 0.0;
        for (size_t j = 0; j < AllPath[i].size(); j++)
        {
            CurTimeSum += AllPath[i][j]->Time;
            for (const auto & CurMes: AllPath[i][j]->MesOut)
            {
                std::shared_ptr<Message> p = CurMes.lock();    
                if (p) 
                {
                    if (p->Dest == AllPath[i][j + 1])
                    {
                        SumMesSize += p->Size;
                    }
                } else 
                {
                    std::cout << "PROBLEMS INITALGORITHM MAIN LOOP" << std::endl;
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
            //std::cout << "NEW DUR = " << NewDur << std::endl;
            for (size_t j = 0; j < AllPath[i].size() - 1; j++)
            {
                for (const auto & CurMes: AllPath[i][j]->MesOut)
                {
                    std::shared_ptr<Message> p = CurMes.lock();    
                    if (p) 
                    {
                        if (!p->NotModify && p->Dest == AllPath[i][j + 1])
                        {
                            p->Dur = std::max(p->Dur, NewDur);
                            if (AllPath[i][j]->Left + AllPath[i][j]->Time + p->Dur + AllPath[i][j + 1]->Time > AllPath[i][j + 1]->Right)
                            {
                                p->Dur = AllPath[i][j + 1]->Right - AllPath[i][j + 1]->Time - AllPath[i][j]->Time - AllPath[i][j]->Left; 
                                p->NotModify = true;
                            }
                        }
                    } else 
                    {
                        std::cout << "PROBLEMS INITALGORITHM MAIN LOOP" << std::endl;
                    }
                }
            }   
        } else if (Mode == 2)
        {
            for (size_t j = 0; j < AllPath[i].size() - 1; j++)
            {
                for (const auto & CurMes: AllPath[i][j]->MesOut)
                {
                    std::shared_ptr<Message> p = CurMes.lock();
                    if (p) 
                    {
                        if (p->Dest == AllPath[i][j + 1])
                        {
                            p->Dur = std::max(p->Dur, (CurMesTime * p->Size) / SumMesSize);
                        }
                    } else 
                    {
                        std::cout << "PROBLEMS INITALGORITHM MAIN LOOP" << std::endl;
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
                std::shared_ptr<Message> p = CurMes.lock();
                if (p) 
                {
                    if (p->Dest == AllPath[i][j + 1])
                    {
                        p->Bandwidth = p->Size / p->Dur;
                        CurBandwidth += p->Bandwidth;
                    }
                } else 
                {
                    std::cout << "PROBLEMS INITALGORITHM MAIN LOOP" << std::endl;
                }
            }
        }   

    }

    CurSystem->CurBLackCoef = BLackCoef(CurSystem->BTotal - CurBandwidth); 
    
    double EarlestTimeForStart;
    for (size_t i = 0; i < AllPath.size(); i++)
    {
        EarlestTimeForStart = 0.0;
        for (size_t j = 0; j < AllPath[i].size(); j++)
        {
            AllPath[i][j]->Left = std::max(AllPath[i][j]->Left, EarlestTimeForStart);
            for (const auto & CurMes: AllPath[i][j]->MesOut)
            {
                std::shared_ptr<Message> p = CurMes.lock();    
                if (p) 
                {
                    if (p->Dest == AllPath[i][j + 1])
                    {
                        EarlestTimeForStart = AllPath[i][j]->Left +
                                              AllPath[i][j]->Time +
                                              p->Dur;
                    }
                } else 
                {
                    std::cout << "PROBLEMS INITALGORITHM MAIN LOOP" << std::endl;
                }
                
            }
        }
    }

    double LatestTimeForEnd;
    for (size_t i = 0; i < AllPath.size(); i++)
    {
        LatestTimeForEnd = 5000000.0; // todo
        for (int j = AllPath[i].size() - 1; j >= 0; j--)
        {
                
            AllPath[i][j]->Right = std::min(AllPath[i][j]->Right, LatestTimeForEnd);
                        
            for (const auto & CurMes: AllPath[i][j]->MesIn)
            {
                std::shared_ptr<Message> p = CurMes.lock();    
                if (p) 
                {
                    if (p->Src == AllPath[i][j - 1])
                    {   
                        LatestTimeForEnd = AllPath[i][j]->Right -
                                           AllPath[i][j]->Time -
                                           p->Dur;
                    }
                } else 
                {
                    std::cout << "PROBLEMS INITALGORITHM MAIN LOOP" << std::endl;
                }
                
            }
        }
    }
    
    //PrintAllPath();  
    
    //CurSystem->PrintSystem();
    return;
}