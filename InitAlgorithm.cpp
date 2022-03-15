#include <algorithm>
#include <limits>
#include "InitAlgorithm.h"


void InitAlgorithm:: SearchPath(std::shared_ptr<Task> CurTask, std::vector<std::shared_ptr<Task>> CurPath, System* CurSystem)
{
    if (CurTask->OutMessage.size() == 0)
    {
        CurPath.push_back(std::shared_ptr<Task>(CurTask));
        AllPath.push_back(CurPath);
        CurPath.pop_back();
        return;    
    }
    //TODO тут одно устловие
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
    double minLeft, maxLeft;
    PrintAllPath();  
    for (size_t i = 0; i < AllPath.size(); i++)
    {
        std::cout << "Path " << i << std::endl;
        minLeft = 0.0;
        for (size_t j = 0; j < AllPath[i].size(); j++)
        {
            //std::cout << "Task : " << j << " Min Left: " << AllPath[i][j]->MinLeft << " cur min left: " << minLeft << std::endl;
            AllPath[i][j]->MinLeft = std::max(AllPath[i][j]->MinLeft, minLeft);
            minLeft = AllPath[i][j]->MinLeft + AllPath[i][j]->Time;
            //std::cout << "Task Min Left " << AllPath[i][j]->MinLeft << std::endl;
        }
        maxLeft = CurSystem->LCMPeriod;
        for (int j = AllPath[i].size() - 1; j >= 0; j--)
        {
            maxLeft -= AllPath[i][j]->Time;
            //std::cout << "Task : " << j << " cur max left: " << maxLeft << " Right - time: " << AllPath[i][j]->Right - AllPath[i][j]->Time << " Right " << AllPath[i][j]->Right << " Time " << AllPath[i][j]->Time << std::endl; 
            AllPath[i][j]->MaxLeft = std::min(AllPath[i][j]->Right - AllPath[i][j]->Time, maxLeft);
            maxLeft = AllPath[i][j]->MaxLeft;
            //std::cout <<"Task Max Left: " << AllPath[i][j]->MaxLeft << " cur max left: " << maxLeft << std::endl;
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
                std::cout << "PROBLEMS" << std::endl;
                exit(1);
            }
            AllPath[i][j]->Right = AllPath[i][j]->Left + AllPath[i][j]->Time;
            
            
            AllPath[i][j]->IsInit = true;
        }
    }
    CurSystem->PrintSystem();
    //TODO профтись просто по задачам
    for (size_t i = 0; i < AllPath.size(); i++)
    {
        std::cout << "Path " << i << std::endl;    
        for (size_t j = 0; j < AllPath[i].size(); j++)
        {
            for (const auto & CurMes: AllPath[i][j]->MesOut)
            {
                std::shared_ptr<Message> p = CurMes.lock();    
                if (p) 
                {
                    if (p->Dest == AllPath[i][j + 1])
                    {
                        std::cout << "Task from " << AllPath[i][j]->Time << " Task to " << AllPath[i][j + 1]->Time << " Time: " << AllPath[i][j + 1]->Left - AllPath[i][j]->Time - AllPath[i][j]->Left << std::endl;
                        p->Dur = AllPath[i][j + 1]->Left - AllPath[i][j]->Right;
                        if (p->Dur == 0)
                        {
                            p->Bandwidth = std::numeric_limits<double>::max();
                        } else 
                        {
                            p->Bandwidth = p->Size / p->Dur;   
                         }
                        
                    }
                } else 
                {
                    std::cout << "PROBLEMS INITALGORITHM MAIN LOOP" << std::endl;
                }    
            }
        }
    }
    //TODO
    //CurSystem->CurBLackCoef = BLackCoef(CurBandwidth, CurSystem->BTotal); 
    
    return;
}


/*void InitAlgorithm:: MainLoop(System* CurSystem)
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
                        if (p->Dur == 0.0)
                        {
                            p->Bandwidth = std::numeric_limits<double>::max();
                            
                        } else {
                            p->Bandwidth = p->Size / p->Dur;
                            CurBandwidth += p->Bandwidth;
                            
                        }
                    }
                } else 
                {
                    std::cout << "PROBLEMS INITALGORITHM MAIN LOOP" << std::endl;
                }
            }
        }   

    }

    CurSystem->CurBLackCoef = BLackCoef(CurBandwidth, CurSystem->BTotal); 
    
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
        LatestTimeForEnd = CurSystem->LCMPeriod;
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
    return;
}*/
