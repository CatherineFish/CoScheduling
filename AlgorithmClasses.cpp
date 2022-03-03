#include <algorithm>

#include "AlgorithmClasses.h"

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
                        if (p->Dest == AllPath[i][j + 1])
                        {
                            p->Dur = std::max(p->Dur, NewDur);
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
    //PrintAllPath();  
    
    //CurSystem->PrintSystem();
    return;
}


MainAlgorithm:: MainAlgorithm(System* CurSystem)
{
    int j = 0;
    for (size_t k = 0; k < CurSystem->SystemTask.size(); k++)
    {
        CurSystem->SystemTask[k]->JobInit = j;
        for (size_t i = 0; i < CurSystem->LCMPeriod / CurSystem->SystemTask[k]->Period; i++)
        {
            //тут надо как-то по-умному копировать
            CurSystem->SystemJob.push_back(std::make_shared<Job>(i));
            CurSystem->SystemJob[j]->Period = CurSystem->SystemTask[k]->Period;
            CurSystem->SystemJob[j]->Time = CurSystem->SystemTask[k]->Time;
            CurSystem->SystemJob[j]->Left = CurSystem->SystemTask[k]->Left + i * CurSystem->SystemTask[k]->Period;
            CurSystem->SystemJob[j]->Right = CurSystem->SystemTask[k]->Right + i * CurSystem->SystemTask[k]->Period;
            CurSystem->SystemJob[j]->InitLeft = CurSystem->SystemJob[j]->Left;
            CurSystem->SystemJob[j]->NumOfTask = k;
            CurSystem->SystemJob[j]->CMessageSize = CurSystem->SystemTask[k]->CMessageSize;
            CurSystem->SystemJob[j]->Slack = CurSystem->SystemJob[j]->Right - CurSystem->SystemJob[j]->Time - CurSystem->PPoint;
            if (i != 0)
            {
                CurSystem->SystemJob[j]->PreviousJob = std::shared_ptr<Job>(CurSystem->SystemJob[j - 1]);                                              
                
            }
            
            j++;                                              
        }

    }
    
    for (const auto & CurMes: CurSystem->SystemMessage)
    {
        for (size_t i = 0; i < CurSystem->LCMPeriod / CurMes->Src->Period; i++)
        {

            CurSystem->SystemJob[CurSystem->SystemTask[CurMes->SrcNum]->JobInit + i]->OutMessage.emplace_back(CurSystem->SystemTask[CurMes->DestNum]->JobInit + i);
            CurSystem->SystemJob[CurSystem->SystemTask[CurMes->DestNum]->JobInit + i]->InMessage.emplace_back(CurSystem->SystemTask[CurMes->SrcNum]->JobInit + i);
            CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>
                      (std::shared_ptr(CurSystem->SystemJob[CurSystem->SystemTask[CurMes->SrcNum]->JobInit + i]), 
                       std::shared_ptr(CurSystem->SystemJob[CurSystem->SystemTask[CurMes->DestNum]->JobInit + i]))] = std::shared_ptr<Message>(CurMes);
        }    
    }

    for (const auto & CurJob: CurSystem->SystemJob)
    {
        Unplanned.push_back(std::shared_ptr(CurJob));
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


void MainAlgorithm:: UpdateBList (System* CurSystem)
{
    double CurScore;
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        for (size_t k = 0; k < CurSystem->SystemPC.size(); k++)
        {
            CurScore = 0.0;
            for (const auto & SendIdx: Unplanned[i]->InMessage)
            {
                if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum != CurSystem->SystemPC[k]->ModNum)
                {
                    CurScore += Unplanned[i]->Period * CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], Unplanned[i])]->Size; //внести коэффициент
                }
            }            
            Unplanned[i]->ListBandwidth[std::shared_ptr<PC>(CurSystem->SystemPC[k])] = CurScore;   
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


void MainAlgorithm:: UpdateLeft(std::shared_ptr<Job> CurJob, System* CurSystem)
{
    for (const auto & SendIdx: CurJob->InMessage)
    {
        if (CurSystem->SystemJob[SendIdx]->IsPlanned)
        {
            CurJob->Left = std::max(std::max(CurJob->Left, CurJob->InitLeft), 
                        CurSystem->SystemJob[SendIdx]->Start + 
                        CurSystem->SystemJob[SendIdx]->Time + 
                        CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->TmpDur ? 
                        CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->Dur : 0.0); //todo    

        }
    }
    return;
}


bool MainAlgorithm:: SortBySlack(std::shared_ptr<Job> i, std::shared_ptr<Job> j)
{
    return i->Slack < j->Slack;
}


int MainAlgorithm::Check(std::shared_ptr<Job> CurJob, std::shared_ptr<PC> PCForPlan, System* CurSystem)
{
    double BSum = 0.0;
    for (const auto & SendIdx: CurJob->InMessage)
    {
        if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC != PCForPlan)
        {
            BSum += CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->Bandwidth;
        }
    }
    if (CurSystem->BTotal <= BSum)
    {
        return 1; // нехватка пропускной способности
    }
    if (CurJob->Right < CurSystem->PPoint + CurJob->Time)
    {
        return 2; //нарушение директивных сроков
    }
    return 0;
}


void MainAlgorithm:: MainLoop(System* CurSystem)
{
    double NewLeft = 0.0;
    int CheckResult;
    bool flag = false, CMesFlag = true;
    UpdateBList(CurSystem);
    UpdateFList(CurSystem->SystemPC);
    UpdateRList();
    for (const auto & CurJob : Unplanned) 
    {
        for (const auto & SendIdx: CurJob->InMessage)
        {
            if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum == CurJob->ListResult.begin()->first->ModNum)
            {
                flag = true;

                CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->TmpDur = 0.0;
            }
        }    

    }
    
    for (const auto & CurJob : Unplanned) 
    {
        
        if (!CurJob->PreviousJob || CurJob->PreviousJob->IsPlanned && CurJob->PreviousJob->JobPC->ModNum == CurJob->ListResult.begin()->first->ModNum)
        {
            CMesFlag = false;
        }
        
        if (CMesFlag) //TODO
        {
            CurSystem->SystemCMessage.push_back(std::make_shared<ContextMessage>(CurJob->PreviousJob, CurJob));
            CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Size = CurJob->CMessageSize;
            CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Bandwidth = CurJob->CMessageSize / CurJob->Slack;//todo
            CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurJob->PreviousJob, CurJob)] = CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1];
            CurJob->InMessage.push_back(find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), CurJob->PreviousJob) - CurSystem->SystemJob.begin());
            //CurJob->MesIn.push_back(std::shared_ptr(CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]));
            //TODO^ нужно удалять неиспользуемые контекстные сообщения
        }
        
        if (flag || CMesFlag)
        {
            UpdateLeft(CurJob, CurSystem);
        }     
    }
    

    for (const auto & CurJob: Unplanned)
    {
        CurJob->Slack = CurJob->Right - CurJob->Time - CurSystem->PPoint;
        if (CurJob->Left >= CurSystem->PPoint)
        {
            QueueForPlan.push_back(std::shared_ptr(CurJob));
        }
    }
    std::sort(QueueForPlan.begin(), QueueForPlan.end(), SortBySlack);
    
    
    std::cout << "Queue For Planning: " << std::endl;
    for (const auto& CurJob: QueueForPlan)
    {
        std::cout << "Job Time: " << CurJob->Time << " JbNum: " << CurJob->NumOfTask << " Job Slack " << CurJob->Slack << std::endl;
    }
    std::cout << std::endl;

    
    if (QueueForPlan.size() == 0)
    {
        //TODO
    }
    CheckResult = Check(QueueForPlan[0], QueueForPlan[0]->ListResult.begin()->first, CurSystem);
    
    if (CheckResult); // TODO
    
    QueueForPlan[0]->JobPC = QueueForPlan[0]->ListResult.begin()->first;
    QueueForPlan[0]->Start = CurSystem->PPoint;
    QueueForPlan[0]->JobPC->PC_PPoint = CurSystem->PPoint + QueueForPlan[0]->Time;


    for (const auto & SendIdx: QueueForPlan[0]->InMessage)
    {
        if (CurSystem->SystemJob[SendIdx]->JobPC != QueueForPlan[0]->JobPC)
        {
            if (CurSystem->SystemJob[SendIdx] == QueueForPlan[0]->PreviousJob) 
            {
                CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], QueueForPlan[0])]->IsPlanned = true;
            
            }
            CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                  std::shared_ptr<Job>>
                                  (CurSystem->SystemJob[SendIdx],
                                  QueueForPlan[0])]->StabilityCoef.Update(-1);//todo
        } else {
            CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                              std::shared_ptr<Job>>
                              (CurSystem->SystemJob[SendIdx],
                              QueueForPlan[0])]->StabilityCoef.Update(+1);//todo
        }
    }

    for (const auto & CurJob: Unplanned)
    {
        auto it = CurJob->InMessage.rbegin();
        //std::cout << "Current Job: " << CurJob->Time << " Num: " << CurJob->Num << std::endl;
        while(it != CurJob->InMessage.rend())
        {
            //std::cout << "IT = " << *it << std::endl;
            //std::cout << "Job: " << CurSystem->SystemJob[*it]->Time << " Job Num " << CurSystem->SystemJob[*it]->Num << std::endl;
        
            if (!CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                       std::shared_ptr<Job>>
                                       (CurSystem->SystemJob[*it], CurJob)]->IsPlanned)
            {
                CurSystem->JobMessage.erase(std::pair<std::shared_ptr<Job>, 
                                            std::shared_ptr<Job>>
                                            (CurSystem->SystemJob[*it], CurJob));
                it = decltype(it)(CurJob->InMessage.erase(std::next(it).base()));
                
                break;
            } else 
            {
                it++;
            }

        }
    }
    
    auto it = CurSystem->SystemCMessage.begin();
    while (it != CurSystem->SystemCMessage.end()){
        if (!(*it)->IsPlanned)
        {
            it = CurSystem->SystemCMessage.erase(it);
        } else
        {
            it++;
        }
    }
    
  
}



void MainAlgorithm:: PrintJobSystem(System* CurSystem)
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
        for (size_t j = 0; j < Unplanned[i]->InMessage.size(); j++)
        {
            std::cout << CurSystem->SystemJob[Unplanned[i]->InMessage[j]]->Time << " " << CurSystem->SystemJob[Unplanned[i]->InMessage[j]]->Num << std::endl;    
        }
        std::cout << std::endl;
        for (size_t j = 0; j < Unplanned[i]->OutMessage.size(); j++)
        {
            std::cout << CurSystem->SystemJob[Unplanned[i]->OutMessage[j]]->Time << " " << CurSystem->SystemJob[Unplanned[i]->OutMessage[j]]->Num << std::endl;    
        }
        std::cout << std::endl;
        std::cout << "List Bandwidth: " << std::endl;
        for (const auto& [key, value] : Unplanned[i]->ListBandwidth) {
            std::cout << '[' << key->Num << "] = " << value << "; " << std::endl;
        }
        std::cout << std::endl;
        std::cout << "List Fill: " << std::endl;
        for (const auto& [key, value] : Unplanned[i]->ListFill) {
            std::cout << '[' << key->Num << "] = " << value << "; " << std::endl;
        }
        std::cout << std::endl;
        std::cout << "List Result: " << std::endl;
        for (const auto& [key, value] : Unplanned[i]->ListResult) {
            std::cout << '[' << key->Num << "] = " << value << "; " << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "======================" << std::endl; 
    return;  
}
