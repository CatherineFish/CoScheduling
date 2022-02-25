

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
    double CurBandwidth = 0.0;
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
        /*
        std::cout << "SUM TIME FOR EXECUTION = " << CurTimeSum << std::endl;
        std::cout << "RIGHT TIME = " << AllPath[i][AllPath[i].size() - 1]->Right << std::endl;
        std::cout << "MESSAGE COUNT = " << MessageCount << std::endl;
        std::cout << "TIME FOR MESSAGES = " << CurMesTime << std::endl;
        */
        
        if (Mode == 1)
        {
            NewDur = CurMesTime / MessageCount;
            //std::cout << "NEW DUR = " << NewDur << std::endl;
            for (size_t j = 0; j < AllPath[i].size() - 1; j++)
            {
                for (const auto & CurMes: AllPath[i][j]->MesOut)
                {
                    //std::cout << CurMes->Dest->Time << " " << AllPath[i][j + 1]->Time << std::endl;
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
                    CurBandwidth += CurMes->Bandwidth;
                }
            }
        }   

    }

    CurSystem->CurBLackCoef = BLackCoef(CurSystem->BTotal - CurBandwidth); 
    //PrintAllPath();  
    
    //CurSystem->PrintSystem();
    return;
}


MainAlgorithm:: MainAlgorithm(System* CurSystem)
{
    int j = 0;
    for (const auto & CurTask: CurSystem->SystemTask)
    {
        CurTask->JobInit = j;
        for (size_t i = 0; i < CurSystem->LCMPeriod / CurTask->Period; i++)
        {
            //тут надо как-то по-умному копировать
            Unplanned.push_back(std::make_shared<Job>(i));
            Unplanned[j]->Period = CurTask->Period;
            Unplanned[j]->Time = CurTask->Time;
            Unplanned[j]->Left = CurTask->Left + i * CurTask->Period;
            Unplanned[j]->Right = CurTask->Right + i * CurTask->Period;
            /*for (const auto & CurMes: CurTask->MesOut)
            {
                Unplanned[j]->MesOut.push_back(std::shared_ptr<Message>(CurMes));
            }
            for (const auto & CurMes: CurTask->MesIn)
            {
                Unplanned[j]->MesIn.push_back(std::shared_ptr<Message>(CurMes));
            }*/
            
            j++;                                              
        }

    }
    /*
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        std::cout << Unplanned[i]->Period << " " << Unplanned[i]->Time << " " << Unplanned[i]->Left << " " << Unplanned[i]->Right << std::endl; 
            
    }
    */
    
    for (const auto & CurMes: CurSystem->SystemMessage)
    {
        for (size_t i = 0; i < CurSystem->LCMPeriod / CurMes->Src->Period; i++)
        {

            Unplanned[CurSystem->SystemTask[CurMes->SrcNum]->JobInit + i]->JobTo.push_back(std::shared_ptr(
                                                                           Unplanned[CurSystem->SystemTask[CurMes->DestNum]->JobInit + i]));
            Unplanned[CurSystem->SystemTask[CurMes->SrcNum]->JobInit + i]->MesOut.push_back(std::shared_ptr<Message>(CurMes));
            Unplanned[CurSystem->SystemTask[CurMes->DestNum]->JobInit + i]->JobFrom.push_back(std::shared_ptr(
                                                                           Unplanned[CurSystem->SystemTask[CurMes->SrcNum]->JobInit + i]));
            Unplanned[CurSystem->SystemTask[CurMes->DestNum]->JobInit + i]->MesIn.push_back(std::shared_ptr<Message>(CurMes));
        }    
    }
    
}

void MainAlgorithm:: UpdateFList (std::vector<std::shared_ptr<PC>> SystemPC)
{
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        for (size_t k = 0; k < SystemPC.size(); k++)
        {
            Unplanned[i]->ListFill[std::shared_ptr<PC>(SystemPC[k])] = SystemPC[k]->PC_PPoint;   
        }
    }
    return;
}

void MainAlgorithm:: UpdateBList (std::vector<std::shared_ptr<PC>> SystemPC)
{
    //std::cout << "Start" << std::endl;
                    
    double CurScore;
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        //std::cout << "Cur Job: " << i << std::endl;
                
        for (size_t k = 0; k < SystemPC.size(); k++)
        {
            //std::cout << "Cur PC: " << k <<  std::endl;
                
            CurScore = 0.0;
                    
            for (size_t j = 0; j < Unplanned[i]->JobFrom.size(); j++)
            {
                /*
                std:: cout << "j = " << j << std::endl;
                std:: cout << Unplanned[i]->JobFrom[i]->IsPlanned << std::endl;    
                std::cout << "Cur Job: " << i << " || Cur PC: " << k << " || Cur Src: " << Unplanned[i]->JobFrom[j]->Time << " " << Unplanned[i]->JobFrom[j]->Num << std::endl;
                */
                if (Unplanned[i]->JobFrom[j]->IsPlanned && Unplanned[i]->JobFrom[j]->JobPC->ModNum != SystemPC[k]->ModNum)
                {
                    //std::cout << "Here" << std::endl;
                    CurScore += Unplanned[i]->Period * Unplanned[i]->MesIn[j]->Size; //внести коэффициент
                }
                //std::cout << "Here" << std::endl;   
            }            
            //Unplanned[i]->ListBandwidth[std::shared_ptr<PC>(SystemPC[k])] = CurScore;   
        }
    }
    return;
}

void MainAlgorithm:: UpdateRList ()
{
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        for (std::pair<std::shared_ptr<PC>, double> CurPC : Unplanned[i]->ListBandwidth) 
        {
            if (CurPC.second < CritLimit)
            {
                Unplanned[i]->ListResult[CurPC.first] = Unplanned[i]->ListFill[CurPC.first];
            }
        }

    }
    return;
}

void MainAlgorithm:: MainLoop(System* CurSystem)
{
    UpdateBList(CurSystem->SystemPC);
    UpdateFList(CurSystem->SystemPC);
    UpdateRList();
}

void MainAlgorithm:: PrintJobSystem()
{
    std::cout << "======== J O B ========" << std::endl; 
    
    std::cout << "--------UNPLANNED--------:" << std::endl;
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        std::cout << std::endl;
        std::cout << "     Job " << Unplanned[i]->Time << std::endl;
        std::cout << "Number: " << Unplanned[i]->Num << std::endl;
        std::cout << "Period: " << Unplanned[i]->Period << std::endl;
        std::cout << "Left: " << Unplanned[i]->Left << std::endl;
        std::cout << "Right: " << Unplanned[i]->Right << std::endl;
        std::cout << "Input Mes From:" << std::endl;
        for (size_t j = 0; j < Unplanned[i]->JobFrom.size(); j++)
        {
            std::cout << Unplanned[i]->JobFrom[j]->Time << " " << Unplanned[i]->JobFrom[j]->Num << std::endl;    
        }
        std::cout << std::endl;
        for (size_t j = 0; j < Unplanned[i]->JobTo.size(); j++)
        {
            std::cout << Unplanned[i]->JobTo[j]->Time << " " << Unplanned[i]->JobTo[j]->Num << std::endl;    
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "======================" << std::endl; 
    return;  
}
