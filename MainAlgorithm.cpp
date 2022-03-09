#include <algorithm>
#include <numeric>
#include "MainAlgorithm.h"
#include "LimitedSearch.h"


MainAlgorithm:: MainAlgorithm(System* CurSystem)
{
    int j = 0;
    for (size_t k = 0; k < CurSystem->SystemTask.size(); k++)
    {
        CurSystem->SystemTask[k]->JobInit = j;
        for (size_t i = 0; i < CurSystem->LCMPeriod / CurSystem->SystemTask[k]->Period; i++)
        {
            //TODO возможно, можно как-то по-умному копировать
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
}


void MainAlgorithm:: UpdateFList (System * CurSystem)
{
    std::vector<int> NewLimit;
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        for (size_t k = 0; k < CurSystem->SystemPC.size(); k++)
        {

            //пересчитываем новый левый директивный интервал работы для конкретного ядра
            //учитываем, что надо ждать сообщения
            //TODO сохранить эти значения, чтобы для планирования не пересчитывать эти значения
            bool flag = false;
            double NewLeft = 0.0;
            for (const auto & SendIdx: Unplanned[i]->InMessage)
            {
                if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum == Unplanned[i]->ListResult.begin()->second->ModNum)
                {
                    flag = true;
                    NewLeft = std::max(NewLeft, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time); 
                } else if (CurSystem->SystemJob[SendIdx]->IsPlanned)
                {
                    flag = true; 
                    NewLeft = std::max(NewLeft, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time + 
                                       CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], Unplanned[i])]->Dur); 
                    
                }
            }
            if (flag)
            {
                Unplanned[i]->ListFill[std::shared_ptr<PC>(CurSystem->SystemPC[k])] = CurSystem->SystemPC[k]->PC_PPoint <= std::max(NewLeft, Unplanned[i]->InitLeft) ? 
                                                                                      0.0 : CurSystem->SystemPC[k]->PC_PPoint;   
            } else 
            {
                //если ядро освобождается раньше того, как можно запланировать работу, но ставим 0
                Unplanned[i]->ListFill[std::shared_ptr<PC>(CurSystem->SystemPC[k])] = CurSystem->SystemPC[k]->PC_PPoint <= Unplanned[i]->Left ? 
                                                                                      0.0 : CurSystem->SystemPC[k]->PC_PPoint;   

            }
            NewLimit.emplace_back(Unplanned[i]->ListFill[std::shared_ptr<PC>(CurSystem->SystemPC[k])]);
        }
    }

    auto PCPointsSum = std::accumulate(NewLimit.begin(), NewLimit.end(), decltype(NewLimit)::value_type(0));
    
    LimitForPC = PCPointsSum != 0.0 ? PCPointsSum / NewLimit.size() : PCPointsSum;
    return;
}


void MainAlgorithm:: UpdateBList (System* CurSystem)
{
    double CurScore;
    bool IsContextMes;
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        for (size_t k = 0; k < CurSystem->SystemPC.size(); k++)
        {
            CurScore = 0.0;
            for (const auto & SendIdx: Unplanned[i]->InMessage)
            {
                if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum != CurSystem->SystemPC[k]->ModNum)
                {
                    CurScore += Unplanned[i]->Period * 
                                CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>
                                                      (CurSystem->SystemJob[SendIdx], Unplanned[i])]->Size *
                                CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>
                                                      (CurSystem->SystemJob[SendIdx], Unplanned[i])]->StabilityCoef.Value;
                                ; //внести коэффициент
                }
            }
            if (Unplanned[i]->PreviousJob && Unplanned[i]->PreviousJob->IsPlanned && Unplanned[i]->PreviousJob->JobPC->ModNum != CurSystem->SystemPC[k]->ModNum)
            {
                IsContextMes = false;
                //предыдущие экземпляр работы не на рассматриваем ядре
                //проверим, нет ли контекстного сообщения уже
                for (const auto & CurMes: CurSystem->SystemCMessage)
                {
                    if (CurMes->Src->NumOfTask == Unplanned[i]->NumOfTask)
                    {
                        //контекстное сообщение есть
                        IsContextMes = true;
                        break;
                    }
                }
                if (!IsContextMes)
                {
                    //если сообщения нет, добавляем пропускную способность в счет ядра
                    CurScore += Unplanned[i]->Period * Unplanned[i]->CMessageSize * CurSystem->CurBLackCoef.Value;     
                }
                
            }
            Unplanned[i]->ListBandwidth[std::shared_ptr<PC>(CurSystem->SystemPC[k])] = CurScore;   
        }
    }
    return;
}


void MainAlgorithm:: UpdateRList ()
{
    /*сначала по пропускной способности, потом по ядрам
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
    */

    //сначала по ядрам, потом по пропускной способности
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        Unplanned[i]->ListResult.clear();
        for (std::pair<std::shared_ptr<PC>, double> CurPC : Unplanned[i]->ListFill) 
        {
            if (CurPC.second <= LimitForPC)
            {
                Unplanned[i]->ListResult.insert(decltype(Unplanned[i]->ListResult)::value_type(Unplanned[i]->ListBandwidth[CurPC.first], CurPC.first));
            }
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
        if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum != PCForPlan->ModNum)
        {
            if (CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>
                                       (CurSystem->SystemJob[SendIdx], CurJob)]->StabilityCoef.Value != 0.0)
            {
                //считаем, сколько пропускной способности будет занято
                BSum += CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->Bandwidth;
            }
        }
    }
    if (CurJob->PreviousJob && CurJob->PreviousJob->IsPlanned && CurJob->PreviousJob->JobPC->ModNum != PCForPlan->ModNum)
    {   
        bool IsContextMes = false;
        //предыдущие экземпляр работы не на рассматриваем ядре
        //проверим, нет ли контекстного сообщения уже
        for (const auto & CurMes: CurSystem->SystemCMessage)
        {
            if (CurMes->Src->NumOfTask == CurJob->NumOfTask)
            {
                //контекстное сообщение есть
                IsContextMes = true;
                break;
            }
        }
        if (!IsContextMes)
        {
            BSum += CurJob->CMessageSize / (CurJob->Left - CurJob->PreviousJob->Start - CurJob->PreviousJob->Time);
        }
        
    }
    if (CurSystem->BTotal <= BSum + CurSystem->CurBand)
    {
        return 1; // нехватка пропускной способности
    }
    if (CurJob->Right < CurSystem->PPoint + CurJob->Time)
    {
        return 2; //нарушение директивных сроков
    }
    //CurSystem->CurBand += BSum;//обновление происходит в коэффициентах
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
        std::vector<std::shared_ptr<Job>> QueueForPlan;
        double NewLeft = 0.0, MinLeft = CurSystem->LCMPeriod;
        int CheckResult, FlagPPoint = -2;
        bool flag = false;
        
        //Что уже запланировано
        std::cout << "PLANNED" << std::endl;
        for (size_t i = 0; i < CurSystem->SystemJob.size(); i++)
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

        //Обновление списков
        UpdateBList(CurSystem);
        UpdateFList(CurSystem);
        UpdateRList();
        
        //Сотрим, что получилось после обновления списков
        PrintJobSystem(CurSystem);
        
        //Обновление левого директивного срока после формирования списков
        std::cout << "UPDATE" << std::endl;
        for (const auto & CurJob : Unplanned) 
        {
            flag = false;
            NewLeft = 0.0;
            std::cout << "Job Time: " << CurJob->Time << " Job Num " << CurJob->Num << std::endl;
            
            for (const auto & SendIdx: CurJob->InMessage)
            {
                if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum == CurJob->ListResult.begin()->second->ModNum)
                {
                    flag = true;
                    std::cout << "Sender " << SendIdx << " was planned on this Module" << std::endl;
                    NewLeft = std::max(NewLeft, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time); 
                } else if (CurSystem->SystemJob[SendIdx]->IsPlanned)
                {
                    flag = true; 
                    std::cout << "Sender " << SendIdx << " was planned on other Module" << std::endl;
                	NewLeft = std::max(NewLeft, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time + 
                                       CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->Dur); 
                	
                }
            }
            if (CurJob->PreviousJob && CurJob->PreviousJob->IsPlanned && CurJob->PreviousJob->JobPC->ModNum != CurJob->ListResult.begin()->second->ModNum)
            {
                //проверим, нет ли контекстного сообщения уже
                
                for (const auto & CurMes: CurSystem->SystemCMessage)
                {
                    if (CurMes->Src->NumOfTask == CurJob->NumOfTask)
                    {
                        //контекстное сообщение есть
                        NewLeft = std::max(NewLeft, CurJob->PreviousJob->Start + CurJob->PreviousJob->Time + CurMes->Dur);
                        break;
                    }
                }
            }
            if (flag)
            {
                std::cout << "Updated Left from " << CurJob->Left << " to " << NewLeft << " OR " << CurJob->InitLeft << std::endl;
                CurJob->Left = std::max(NewLeft, CurJob->InitLeft);    
            }
            std::cout << "Want to plan: " << CurJob->Left << " Can Plan: " << CurJob->ListResult.begin()->second->PC_PPoint << std::endl; 
            CurJob->Left = std::max(CurJob->ListResult.begin()->second->PC_PPoint, CurJob->Left);
        }

        //Смотрим, как обновились левые директивные сроки
        PrintJobSystemWithoutLists(CurSystem);
        
        while (FlagPPoint < 0)
        {
            MinLeft = CurSystem->LCMPeriod;
            std::cout << "MAKE Queue" << std::endl;
            std::cout << "Cur PPoint = " << CurSystem->PPoint << std::endl;
            for (const auto & CurJob: Unplanned)
            {
                MinLeft = std::min(CurJob->Left, MinLeft);
                std::cout << "Cur Job: " << CurJob->Time << " NUM " << CurJob->Num;
                CurJob->Slack = CurJob->Right - CurJob->Time - CurSystem->PPoint;
                std::cout << " Slack " << CurJob->Slack << std::endl;
                if (CurJob->Slack < 0)
                {
                    if (Unplanned[0] == CurJob)
                    {
                        //TODO нужно разбирать расписание от предыдущей работы
                    }
                    CurSystem->PPoint = CurJob->Right - CurJob->Time;
                    QueueForPlan.clear();
                    QueueForPlan.push_back(std::shared_ptr(CurJob));
                    // работа с отрицательным слаком поставлена первой, выходим из цикла и её хотим планировать
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
                //нет работ для планирования из-за неправильной точки планирования
                //скорее всего точка слишком слева
                CurSystem->PPoint = MinLeft;
                QueueForPlan.clear();
                FlagPPoint = -2;
            }
        }

        //сортируем по слаку и смотри на очередь для планирования
        std::sort(QueueForPlan.begin() + FlagPPoint, QueueForPlan.end(), SortBySlack);
        std::cout << "Queue For Planning: " << std::endl;
        for (const auto& CurJob: QueueForPlan)
        {
            std::cout << "Job Time: " << CurJob->Time << " JbNum: " << CurJob->NumOfTask << " Job Slack " << CurJob->Slack << std::endl;
        }
        std::cout << std::endl;
    
        //что хотим запланировать
        std::cout << "Planning" << std::endl;
        std::cout << "Job Time " << QueueForPlan[0]->Time << " JobNum " << QueueForPlan[0]->Num << " PC: " << QueueForPlan[0]->ListResult.begin()->second->Num << std::endl;
        std::cout << "Point for plan: " << CurSystem->PPoint;
        
        //проверяем, что можно запланировать
        CheckResult = Check(QueueForPlan[0], QueueForPlan[0]->ListResult.begin()->second, CurSystem);
        std::cout << "Check Result: " << CheckResult << std::endl;

        auto CurPC = QueueForPlan[0]->ListResult.begin()->second;
        if (CheckResult == 2) // TODO запуск ограниченного перебора
        {
            LimitedSearch CurLimitedSerch(10);
            CurPC = CurLimitedSerch.MainLoop(1, QueueForPlan[0], Planned, CurSystem); 
        }
        //Планируем
        QueueForPlan[0]->JobPC = std::shared_ptr<PC>(CurPC);
        QueueForPlan[0]->Start = CurSystem->PPoint;
        QueueForPlan[0]->JobPC->PC_PPoint = CurSystem->PPoint + QueueForPlan[0]->Time;
        QueueForPlan[0]->IsPlanned = true;

        //Если предыдущий экземпляр работы запланирован на дургой модуль, формируем контекстное сообщение, если его нет
        if (QueueForPlan[0]->PreviousJob && QueueForPlan[0]->PreviousJob->IsPlanned && 
            QueueForPlan[0]->JobPC->ModNum != QueueForPlan[0]->PreviousJob->JobPC->ModNum)
        {
            bool IsContextMes = false;
            for (const auto & CurMes: CurSystem->SystemCMessage)
            {
                if (CurMes->Src->NumOfTask == QueueForPlan[0]->NumOfTask)
                {
                    //контекстное сообщение есть
                    IsContextMes = true;
                    break;
                }
            }
            if (!IsContextMes)
            {
                //контекстного сообщения нет - создаём
                CurSystem->SystemCMessage.push_back(std::make_shared<ContextMessage>(QueueForPlan[0]->PreviousJob, QueueForPlan[0]));
                auto CurMes = CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1];
                CurMes->Size = QueueForPlan[0]->CMessageSize;
                CurMes->Dur = QueueForPlan[0]->Left -
                              QueueForPlan[0]->PreviousJob->Start -
                              QueueForPlan[0]->PreviousJob->Time;            
                CurMes->Bandwidth = QueueForPlan[0]->CMessageSize / 
                                    (QueueForPlan[0]->Left -
                                    QueueForPlan[0]->PreviousJob->Start - 
                                    QueueForPlan[0]->PreviousJob->Time);
                CurSystem->CurBLackCoef.Update(false, CurMes->Bandwidth, CurMes->StabilityCoef.CountNotPlanned, CurMes->StabilityCoef.CountInPeriod);
                
                CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                      std::shared_ptr<Job>>
                                      (QueueForPlan[0]->PreviousJob, QueueForPlan[0])] = CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1];
                    
            }
            //чтобы не перегружать CurSystem->JobMessage, там только одно контекстное сообщение 
                          
        }
    

        //обновление коэффициентов для сообщений
        for (const auto & SendIdx: QueueForPlan[0]->InMessage)
        {
            std::shared_ptr<Message> CurMes = CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                                                    std::shared_ptr<Job>>
                                                                    (CurSystem->SystemJob[SendIdx],
                                                                    QueueForPlan[0])];

            if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum != QueueForPlan[0]->JobPC->ModNum)
            {
                if (CurMes->StabilityCoef.Value != 0) 
                {
                    CurSystem->CurBand += CurMes->Bandwidth;
                }  
                CurMes->StabilityCoef.Update(true);
                CurSystem->CurBLackCoef.Update(true, CurMes->Bandwidth, CurMes->StabilityCoef.CountNotPlanned, CurMes->StabilityCoef.CountInPeriod);
            } else {
                CurMes->StabilityCoef.Update(false);
                CurSystem->CurBLackCoef.Update(false, CurMes->Bandwidth, CurMes->StabilityCoef.CountNotPlanned, CurMes->StabilityCoef.CountInPeriod);
            }
        }
        
        //обновляем список незапланированных работ
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
        
        //обновляем ядро, на которое поставили работу
        //хз, зачем так сложно было
        //auto ItPCForPlan = find(CurSystem->SystemPC.begin(), CurSystem->SystemPC.end(), QueueForPlan[0]->JobPC);	
        QueueForPlan[0]->JobPC->PlannedOnPC.emplace_back(find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), QueueForPlan[0]) - CurSystem->SystemJob.begin());
        
        //обновляем список запланированных работ
        Planned.push_back(std::shared_ptr<Job>(QueueForPlan[0]));
        
        //обновляем точку планирования
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


void MainAlgorithm:: PrintJobSystemWithoutLists(System* CurSystem)
{
    std::cout << "======== J O B ========" << std::endl; 
    
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        std::cout << std::endl;
        std::cout << "     Job " << Unplanned[i]->Time << std::endl;
        std::cout << "Number: " << Unplanned[i]->Num << std::endl;
        std::cout << "Period: " << Unplanned[i]->Period << std::endl;
        std::cout << "Left: " << Unplanned[i]->Left << std::endl;
        std::cout << "Right: " << Unplanned[i]->Right << std::endl;
    }
    std::cout << std::endl;
    std::cout << "======================" << std::endl; 
    return;  
}
