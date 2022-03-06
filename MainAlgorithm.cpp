#include <algorithm>

#include "MainAlgorithm.h"


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
            CurSystem->SystemJob[j]->InitLeft = CurSystem->SystemTask[k]->InitLeft + i * CurSystem->SystemTask[k]->Period;
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
    PrintJobSystem(CurSystem);
    
}


void MainAlgorithm:: UpdateFList (std::vector<std::shared_ptr<PC>> SystemPC)
{
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        for (size_t k = 0; k < SystemPC.size(); k++)
        {
            Unplanned[i]->ListFill[std::shared_ptr<PC>(SystemPC[k])] = SystemPC[k]->PC_PPoint < Unplanned[i]->Left ? 0.0 : SystemPC[k]->PC_PPoint;   
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
            if (Unplanned[i]->PreviousJob && Unplanned[i]->PreviousJob->IsPlanned && Unplanned[i]->PreviousJob->JobPC->ModNum != CurSystem->SystemPC[k]->ModNum)
            {
                CurScore += Unplanned[i]->Period * Unplanned[i]->CMessageSize ; //todo * CurSystem->CurBLackCoef
                //Unplanned[i]->Left = std::max(Unplanned[i]->Left, Unplanned[i]->PreviousJob->Start + Unplanned[i]->PreviousJob->Time + CurJob->Slack);
                Unplanned[i]->IsCorrected = Unplanned[i]->PreviousJob->Start + Unplanned[i]->PreviousJob->Time;
                double DurWant = Unplanned[i]->Left - (Unplanned[i]->PreviousJob->Start + Unplanned[i]->PreviousJob->Time);
                double BandNeed = Unplanned[i]->CMessageSize / DurWant;
                double BandHave = CurSystem->CurBLackCoef.TakeBand(BandNeed);
                Unplanned[i]->BandForCMes = BandHave;
                Unplanned[i]->IsCorrected += Unplanned[i]->CMessageSize / BandHave;
            } else {
                Unplanned[i]->IsCorrected = 0.0;
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
        Unplanned[i]->ListResult.clear();
        for (std::pair<std::shared_ptr<PC>, double> CurPC : Unplanned[i]->ListBandwidth) 
        {
            if (CurPC.second < CritLimit)
            {
                Unplanned[i]->ListResult.insert(decltype(Unplanned[i]->ListResult)::value_type(Unplanned[i]->ListFill[CurPC.first], CurPC.first));
            }
        }

    }
    return;
}


void MainAlgorithm:: UpdateLeft(std::shared_ptr<Job> CurJob, System* CurSystem)
{
    std::cout << "Update Left " <<std::endl;
    std::cout << "Job Time: " << CurJob->Time << " Job Num " << CurJob->Num << std::endl; 

    for (const auto & SendIdx: CurJob->InMessage)
    {
        if (CurSystem->SystemJob[SendIdx]->IsPlanned)
        {
            double tmp = CurSystem->SystemJob[SendIdx]->Start + 
                        CurSystem->SystemJob[SendIdx]->Time + 
                        ((CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->TmpDur == 1.0) ? 
                        CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->Dur : 0.0);
            double res = (CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->TmpDur == 1.0) ? 
                        CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->Dur : 0.0;
            std::cout << CurSystem->SystemJob[SendIdx]->Start << " " << res << " " << CurSystem->SystemJob[SendIdx]->Time << " " << tmp <<std::endl;
            std::cout << CurJob->InitLeft << std::endl;
            CurJob->Left = std::max(CurJob->InitLeft, 
                        CurSystem->SystemJob[SendIdx]->Start + 
                        CurSystem->SystemJob[SendIdx]->Time + 
                        ((CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->TmpDur == 1.0) ? 
                        CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->Dur : 0.0)); //todo    
            std::cout << "New Left = " << CurJob->Left << std::endl;
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

double MainAlgorithm:: UpdatePPoint(System* CurSystem)
{
    double NewPoint = CurSystem->LCMPeriod;
    for (const auto & CurPC: CurSystem->SystemPC)
    {
        if (CurPC->PlannedOnPC.size() == 0) {
            NewPoint = 0.0;
            break;
        }
        NewPoint = std::min(NewPoint, CurSystem->SystemJob[CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1]]->Start + 
                            CurSystem->SystemJob[CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1]]->Time);
    }
    return NewPoint;
}


void MainAlgorithm:: MainLoop(System* CurSystem)
{
    while (Unplanned.size())
    {
        CurSystem->PrintMessages();
        std::cout << "Current size: " << Unplanned.size() << std::endl;
        
        std::vector<std::shared_ptr<Job>> QueueForPlan;
        double NewLeft = 0.0, MinLeft = 100000000.0;//TODO MIN
        int CheckResult, FlagPPoint = -2;
        bool flag = false, CMesFlag = true;
        UpdateBList(CurSystem);
        UpdateFList(CurSystem->SystemPC);
        UpdateRList();
        std::cout << "PLANNED" << std::endl;
        
        for (int i = 0; i < CurSystem->SystemJob.size(); i++)
        {
            if (CurSystem->SystemJob[i]->IsPlanned)
            {
                std::cout << "Job Time: " << CurSystem->SystemJob[i]->Time << 
                             " Job Num " << CurSystem->SystemJob[i]->Num << 
                             " PC " << CurSystem->SystemJob[i]->JobPC->Num << 
                             " " << CurSystem->SystemJob[i]->JobPC->ModNum << 
                             " NUM: " << i << std::endl;

            
                
            }
        }
        std::cout << "UPDATE" << std::endl;
        
        for (const auto & CurJob : Unplanned) 
        {
            flag = false;
            NewLeft = 0.0;
            std::cout << "Job Time: " << CurJob->Time << " Job Num " << CurJob->Num << std::endl; flag = false;
            CMesFlag = true;
            for (const auto & SendIdx: CurJob->InMessage)
            {
                std::cout << SendIdx << std::endl;
                //std::cout << CurSystem->SystemJob[SendIdx]->JobPC->ModNum << std::endl;
            	
                if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum == CurJob->ListResult.begin()->second->ModNum)
                {
                    flag = true;
                    
                    std::cout << "OKKKK" << std::endl;
                    NewLeft = std::max(NewLeft, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time); 
                    //CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->TmpDur = 0.0;
                } else if (CurSystem->SystemJob[SendIdx]->IsPlanned)
                {
                    flag = true;
                    
                	NewLeft = std::max(NewLeft, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time + 
                                       CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->Dur); 
                	
                }
            }
            std::cout << CurJob->IsCorrected << " " << NewLeft << " " << flag <<  std::endl; 
            if (flag)
            {
                std::cout << "HELLo" << std::endl;
                if (CurJob->IsCorrected != 0.0)
                {
                    std::cout << "HERE" << std::endl;
                    CurJob->Left = std::max(CurJob->IsCorrected, NewLeft); 
                } else {
                    CurJob->Left = NewLeft;
                
                }    
            }
            std::cout << CurJob->Left << " " << CurJob->ListResult.begin()->second->PC_PPoint << std::endl; 
            CurJob->Left = CurJob->ListResult.begin()->second->PC_PPoint > CurJob->Left ?
                                            CurJob->ListResult.begin()->second->PC_PPoint :
                                            CurJob->Left;
                
            /*if (!CurJob->PreviousJob || !CurJob->PreviousJob->IsPlanned || (CurJob->PreviousJob->IsPlanned && CurJob->PreviousJob->JobPC->ModNum == CurJob->ListResult.begin()->second->ModNum))
            {
                std::cout << "NO CMESSAGE NEED" << std::endl;
                CMesFlag = false;
            }   
            if (CMesFlag) 
            {
                CurSystem->SystemCMessage.push_back(std::make_shared<ContextMessage>(CurJob->PreviousJob, CurJob));
                CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Size = CurJob->CMessageSize;
                CurJob->Slack = CurJob->Right - CurJob->Time - CurSystem->PPoint;
                CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Bandwidth = CurJob->CMessageSize / CurJob->Slack;
                CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurJob->PreviousJob, CurJob)] = CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1];
                CurJob->InMessage.push_back(find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), CurJob->PreviousJob) - CurSystem->SystemJob.begin());
            }
            */
            /*
            if (flag)
            {
                UpdateLeft(CurJob, CurSystem);
            }*/
               
        }
        PrintJobSystem(CurSystem);
        
        while (FlagPPoint < 0)
        {
            MinLeft = 100000.0;//todo
            std::cout << "MAke Queue" << std::endl;
            std::cout << "Cur PPoint = " << CurSystem->PPoint << std::endl;
            for (const auto & CurJob: Unplanned)
            {
                MinLeft = std::min(MinLeft, CurJob->ListResult.begin()->second->PC_PPoint > CurJob->Left ?
                                            CurJob->ListResult.begin()->second->PC_PPoint :
                                            CurJob->Left);
                std::cout << "Cur Job: " << CurJob->Time << " NUM " << CurJob->Num;
                CurJob->Slack = CurJob->Right - CurJob->Time - CurSystem->PPoint;
                std::cout << " Slack " << CurJob->Slack << std::endl;
                if (CurJob->Slack < 0)
                {
                    CurSystem->PPoint = CurJob->Right - CurJob->Time;
                    QueueForPlan.clear();
                    QueueForPlan.push_back(std::shared_ptr(CurJob));
                    FlagPPoint = -1;
                    break;
                }
                if (CurJob->Left <= CurSystem->PPoint)
                {
                    std::cout << "Add in Queue" << std::endl;
                    QueueForPlan.push_back(std::shared_ptr(CurJob));
                }
            }
            if (FlagPPoint == -2) FlagPPoint = 0;
            FlagPPoint *= -1;
            if (QueueForPlan.size() == 0)
            {

                CurSystem->PPoint = MinLeft;
                QueueForPlan.clear();
                FlagPPoint = -2;

            }
        }
        std::sort(QueueForPlan.begin() + FlagPPoint, QueueForPlan.end(), SortBySlack);
        
        
        std::cout << "Queue For Planning: " << std::endl;
        for (const auto& CurJob: QueueForPlan)
        {
            std::cout << "Job Time: " << CurJob->Time << " JbNum: " << CurJob->NumOfTask << " Job Slack " << CurJob->Slack << std::endl;
        }
        std::cout << std::endl;
    
        std::cout << "Planning" << std::endl;
        std::cout << "Job Time " << QueueForPlan[0]->Time << " JobNum " << QueueForPlan[0]->Num << " PC: " << QueueForPlan[0]->ListResult.begin()->second->Num << std::endl;
        std::cout << "Point for plan: " << CurSystem->PPoint;
        
        CheckResult = Check(QueueForPlan[0], QueueForPlan[0]->ListResult.begin()->second, CurSystem);
        
        if (CheckResult); // TODO
        std::cout << "Check Result: " << CheckResult << std::endl;


        QueueForPlan[0]->JobPC = std::shared_ptr<PC>(QueueForPlan[0]->ListResult.begin()->second);
        std::cout << "Point for plan: " << CurSystem->PPoint;
        
        QueueForPlan[0]->Start = CurSystem->PPoint;
        std::cout << "CHEC PLAN JOB " << QueueForPlan[0]->Time << " PC: " << QueueForPlan[0]->JobPC->Num << " " << QueueForPlan[0]->JobPC->ModNum << std::endl; 
        QueueForPlan[0]->JobPC->PC_PPoint = CurSystem->PPoint + QueueForPlan[0]->Time;
        QueueForPlan[0]->IsPlanned = true;
        if (QueueForPlan[0]->PreviousJob && QueueForPlan[0]->JobPC->ModNum != QueueForPlan[0]->PreviousJob->JobPC->ModNum)
        {
            CurSystem->SystemCMessage.push_back(std::make_shared<ContextMessage>(QueueForPlan[0]->PreviousJob, QueueForPlan[0]));
            CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Size = QueueForPlan[0]->CMessageSize;
            CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Bandwidth = QueueForPlan[0]->BandForCMes;
            CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Dur = QueueForPlan[0]->CMessageSize / QueueForPlan[0]->BandForCMes;
            CurSystem->CurBLackCoef.Update(false, CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]);
            
            CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                  std::shared_ptr<Job>>
                                  (QueueForPlan[0]->PreviousJob, QueueForPlan[0])] = CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1];
                
        }
    
        for (const auto & SendIdx: QueueForPlan[0]->InMessage)
        {
            if (CurSystem->SystemJob[SendIdx]->JobPC != QueueForPlan[0]->JobPC)
            {
                /*if (CurSystem->SystemJob[SendIdx] == QueueForPlan[0]->PreviousJob) 
                {
                    CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], QueueForPlan[0])]->IsPlanned = true;
                
                }
                */
                std::shared_ptr<Message> CurMes = CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                                                 std::shared_ptr<Job>>
                                                                 (CurSystem->SystemJob[SendIdx],
                                                                 QueueForPlan[0])];
                CurMes->StabilityCoef.Update(-1);//todo
                CurSystem->CurBLackCoef.Update(false, CurMes);
            } else {
                std::shared_ptr<Message> CurMes = CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                                  std::shared_ptr<Job>>
                                                  (CurSystem->SystemJob[SendIdx],
                                                  QueueForPlan[0])];
                CurMes->StabilityCoef.Update(+1);//todo
                CurSystem->CurBLackCoef.Update(true, CurMes);
                
            }
        }
    
        /*
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
        */

        auto UnPlannedIt = Unplanned.begin();
        while (UnPlannedIt != Unplanned.end()){
            if (*UnPlannedIt == QueueForPlan[0])
            {
                UnPlannedIt = Unplanned.erase(UnPlannedIt);
                break;
            } else
            {
                UnPlannedIt++;
            }
        }
        
        auto ItPCForPlan = find(CurSystem->SystemPC.begin(), CurSystem->SystemPC.end(), QueueForPlan[0]->JobPC);
        	
        (*ItPCForPlan)->PlannedOnPC.emplace_back(find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), QueueForPlan[0]) - CurSystem->SystemJob.begin());
        Planned.push_back(std::shared_ptr<Job>(QueueForPlan[0]));
        CurSystem->PPoint = UpdatePPoint(CurSystem);
        
    }
    return;
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
            std::cout << '[' <<  key  << "] = " << value->Num << "; " << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "======================" << std::endl; 
    return;  
}
