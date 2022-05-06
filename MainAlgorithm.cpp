#include <algorithm>
#include <numeric>
#include "MainAlgorithm.h"
#include "LimitedSearch.h"


MainAlgorithm:: MainAlgorithm(System* CurSystem, int Mood_) : Mood(Mood_)
{
    int j = 0;
    for (size_t k = 0; k < CurSystem->SystemTask.size(); k++)
    {
        CurSystem->SystemTask[k]->JobInit = j;
        for (size_t i = 0; i < CurSystem->LCMPeriod / CurSystem->SystemTask[k]->Period; i++)
        {
            std::cout << "LOOOK FOR PROBLEM TASK: " << CurSystem->SystemTask[k]->Left << " ";
            CurSystem->SystemJob.push_back(std::make_shared<Job>(CurSystem->SystemTask[k]->Period,
                                                                 CurSystem->SystemTask[k]->Time,
                                                                 CurSystem->SystemTask[k]->Left + i * CurSystem->SystemTask[k]->Period,
                                                                 CurSystem->SystemTask[k]->Right + i * CurSystem->SystemTask[k]->Period,
                                                                 CurSystem->SystemTask[k]->CMessageSize,
                                                                 i, k));
            std::cout << "JOB: " << CurSystem->SystemJob[j]->Left << " ";
            
            CurSystem->SystemJob[j]->InitRight = CurSystem->SystemTask[k]->InitRight + i * CurSystem->SystemTask[k]->Period;
            CurSystem->SystemJob[j]->InitLeft = CurSystem->SystemTask[k]->InitLeft + i * CurSystem->SystemTask[k]->Period;
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


void MainAlgorithm:: UpdateFList (System * CurSystem)
{
    std::vector<int> NewLimit;
    for (size_t i = 0; i < Unplanned.size(); i++)
    {
        Unplanned[i]->NewLimitForPlan.clear();
        Unplanned[i]->isUpdated.clear();
        for (size_t k = 0; k < CurSystem->SystemPC.size(); k++)
        {

            //пересчитываем новый левый директивный интервал работы для конкретного ядра
            //учитываем, что надо ждать сообщения
            bool flag = false;
            double NewLeft = 0.0;
            for (const auto & SendIdx: Unplanned[i]->InMessage)
            {
                if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum == CurSystem->SystemPC[k]->ModNum)
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
            if (Unplanned[i]->PreviousJob && Unplanned[i]->PreviousJob->IsPlanned && Unplanned[i]->PreviousJob->JobPC->ModNum != Unplanned[i]->ListResult.begin()->second->ModNum)
            {
                //проверим, нет ли контекстного сообщения уже
                for (const auto & CurMes: CurSystem->SystemCMessage)
                {
                    if (CurMes->Src->NumOfTask == Unplanned[i]->NumOfTask)
                    {
                        //контекстное сообщение есть
                        NewLeft = std::max(NewLeft, Unplanned[i]->PreviousJob->Start + Unplanned[i]->PreviousJob->Time + CurMes->Dur);
                        break;
                    }
                }
            }
            Unplanned[i]->NewLimitForPlan.push_back(NewLeft);
            Unplanned[i]->isUpdated.push_back(flag);
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
    std::vector<int> NewLimit;
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
                                                      (CurSystem->SystemJob[SendIdx], Unplanned[i])]->StabilityCoef.Value *
                                CurSystem->CurBLackCoef.Value;
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
            NewLimit.emplace_back(CurScore);   
        }
    }
    auto PCPointsSum = std::accumulate(NewLimit.begin(), NewLimit.end(), decltype(NewLimit)::value_type(0));
    
    CritLimit = PCPointsSum != 0.0 ? PCPointsSum / NewLimit.size() : PCPointsSum;
    return;
}


void MainAlgorithm:: UpdateRList ()
{
    if (Mood == 1)
    {
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
    } else if (Mood == 2)
    {
        // сначала по пропускной способности, потом по ядрам
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
    } else 
    {
        std::cout << "Invalid Mood for Result List" << std::endl;
        exit(1);
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
    if (CurSystem->BTotal < BSum + CurSystem->CurBand)
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
    for (const auto & CurJob: Unplanned)
    {
        NewPoint = std::min(NewPoint, CurJob->Left);
    }
    
    /*
    //смысла ориентироваться на самое раннее время на ядрах нет
    //работы планируется на свое самое ранее время
    //если оно больше, чем на ядрах - будем двигать точку планирования
    //если оно меньше - все равно не можем пдланировать
    for (const auto & CurPC: CurSystem->SystemPC)
    {
        if (CurPC->PlannedOnPC.size() == 0) {
            NewPoint = 0.0;
            break;
        }
        NewPoint = std::min(NewPoint, CurSystem->SystemJob[CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1]]->Start + 
                            CurSystem->SystemJob[CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1]]->Time);
    }
    */
    return NewPoint;
}


void MainAlgorithm:: MainLoop(System* CurSystem)
{
    int repeat = 0;
    while (Unplanned.size())
    {
        std::vector<std::shared_ptr<Job>> QueueForPlan;
        double NewLeft = 0.0, MinLeft = CurSystem->LCMPeriod;
        int CheckResult, FlagPPoint = -2;
        bool flag = false;
        
        //Что уже запланировано
        std::cout << "PLANNED" << std::endl;
        std::cout << "Current band = " << CurSystem->CurBand << std::endl;
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
            if (CurJob->isUpdated[CurJob->ListResult.begin()->second->Num])
            {
                std::cout << "Updated Left from " << CurJob->Left << " to " << CurJob->NewLimitForPlan[CurJob->ListResult.begin()->second->Num] << " OR " << CurJob->InitLeft << std::endl;
                CurJob->Left = std::max(CurJob->NewLimitForPlan[CurJob->ListResult.begin()->second->Num], CurJob->InitLeft);    
            }
            std::cout << "Want to plan: " << CurJob->Left << " Can Plan: " << CurJob->ListResult.begin()->second->PC_PPoint << std::endl; 
            CurJob->Left = std::max(CurJob->ListResult.begin()->second->PC_PPoint, CurJob->Left);
        }

        //Смотрим, как обновились левые директивные сроки
        PrintJobSystemWithoutLists();
        
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
                    // TODO на стресс-тест : эта работа встанет самой первой, попадет в перебор - то, чего мы и хотим
                    // поправлять тут бессмысленно - левый директивный срок у задачи такой по первому ядру в результируеющем списке
                    // проблемы в нем. Они решаются как раз в ограниченном переборе 
                    /*
                    // нужно обновлять точку планирования
                    if (Unplanned[0] == CurJob)
                    {
                        //нужно разбирать расписание от предыдущей работы
                    }
                    CurSystem->PPoint = CurJob->Right - CurJob->Time;
                    QueueForPlan.clear();
                    QueueForPlan.push_back(std::shared_ptr(CurJob));
                    // работа с отрицательным слаком поставлена первой, выходим из цикла и её хотим планировать
                    FlagPPoint = -1;
                    break;
                    */
                }
                if (CurJob->Left <= CurSystem->PPoint)
                {
                    std::cout << "Add in Queue" << std::endl;
                    QueueForPlan.push_back(std::shared_ptr(CurJob));
                }
            }
            if (FlagPPoint == -2) FlagPPoint = 0;
            // FlagPPoint *= -1;
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
        if (CheckResult == 1)
        {
            std::cout << "here problem with bandwidth" << std::endl;
            LimitedSearch CurLimitedSerch(Mood, 10, 2, true);
            std::cout << "limited serch object" << std::endl;
            CurPC = CurLimitedSerch.MainLoop(0, QueueForPlan[0], Planned, CurSystem, Unplanned);
            if (!CurLimitedSerch.isSucces)
            {
                std::cout << "help" << std::endl;
                Unplanned.clear();
                for (const auto & CurJob: CurSystem->SystemJob)
                {
                    Unplanned.push_back(std::shared_ptr(CurJob));
                }
                for (const auto & CurJob: Planned)
                {
                    CurJob->IsPlanned = false;
                    CurJob->ListBandwidth.clear();
                    CurJob->ListFill.clear();
                    CurJob->ListResult.clear();
                }
                for (const auto & CurPC: CurSystem->SystemPC)
                {
                    CurPC->PC_PPoint = 0;
                }
                for (const auto & CurMes : CurSystem->SystemMessage)
                {
                    CurMes->StabilityCoef.Value = 1.0;
                    CurMes->StabilityCoef.NumOfPlanned = -1;
                    CurMes->StabilityCoef.CountNotPlanned = 0;
                }
                Planned.clear();
                CurSystem->SystemCMessage.clear();
                CurSystem->CurBLackCoef.ReUpdate();
                continue;
            }
                 
        } else if (CheckResult == 2)
        {
            LimitedSearch CurLimitedSerch(Mood, 10, 2);
            CurPC = CurLimitedSerch.MainLoop(0, QueueForPlan[0], Planned, CurSystem, Unplanned); 
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
    for (const auto & CurMes: CurSystem->SystemMessage)
    {
        if (CurMes->StabilityCoef.CountNotPlanned == CurMes->StabilityCoef.CountInPeriod)
        {
           CurMes->ResultPlanned = false; 
        }
    }
    for (const auto & CurMes: CurSystem->SystemCMessage)
    {
        if (CurMes->StabilityCoef.CountNotPlanned == CurMes->StabilityCoef.CountInPeriod)
        {
            std::cout << "CONTEXT MESSAGE NOT PLANNED" << std::endl;
            return;
            CurMes->ResultPlanned = false; 
        } 

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


void MainAlgorithm:: PrintJobSystemWithoutLists()
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

void MainAlgorithm:: ResultCheck(System * CurSystem)
{

    if (Unplanned.size() != 0) 
    {
        std::cout << "Problem: not empty unplanned vector" << std::endl;
        return;
    }
    if (Planned.size() != CurSystem->SystemJob.size()) 
    {
        std::cout << "Problem: not all jobs are planned" << std::endl;
        return;
    }
    double SumBand = 0.0;
    for (const auto & [Jobs, Mes] : CurSystem->JobMessage) {
        if (Mes->ResultPlanned && (Jobs.first->JobPC->ModNum == Jobs.second->JobPC->ModNum)) 
        {
            std::cout << "Problem: message for jobs on one module" << std::endl;
            std::cout << "Mes: " << Mes->Size << std::endl;
            
            //return;
        } else {
            std::cout << "OK Mes: " << Mes->Size << std::endl;    
        }
        if (!Mes->ResultPlanned && (Jobs.first->JobPC->ModNum != Jobs.second->JobPC->ModNum)) 
        {
            std::cout << "Problem: no message for jobs on different module" << std::endl;
            std::cout << "Job SRC: " << Jobs.first->Start << " Time : " << Jobs.first->Time <<
                         " Job DEST: " << Jobs.second->Time << " Time : " << Jobs.second->Start << std::endl;
            return;
        }
            
        if (Mes->ResultPlanned)
        {
            SumBand += Mes->Bandwidth;
        }
        if (Mes->ResultPlanned && Jobs.first->JobPC->ModNum != Jobs.second->JobPC->ModNum && Jobs.first->Start + Jobs.first->Time + Mes->Dur > Jobs.second->Start)
        {
            std::cout << "Problem: message arrives too late" << std::endl;
            std::cout << "Job SRC: " << Jobs.first->Time << " Time : " << Jobs.first->Start <<
                         " Job DEST: " << Jobs.second->Time << " Time : " << Jobs.second->Start << std::endl;
            return;
        }
    }
    for (const auto & Mes : CurSystem->SystemCMessage) {
        if (Mes->Src->JobPC->ModNum == Mes->Dest->JobPC->ModNum) 
        {
            std::cout << "Problem: context message for jobs on one module" << std::endl;
            return;
        }
        if (Mes->Src->Start + Mes->Src->Time + Mes->Dur > Mes->Dest->Start)
        {
            std::cout << "Problem: context message arrives too late" << std::endl;
            return;
        }
        SumBand += Mes->Bandwidth;
    }
    if (SumBand > CurSystem->BTotal) 
    {
        std::cout << "Problem: messages have too much bandwidth" << std::endl;
        return;
    }
    for (const auto & Job : CurSystem->SystemJob)
    {
        if (Job->Start + Job->Time > Job->InitRight) 
        {
            std::cout << "Problem: job ends too late" << std::endl;
            std::cout << "Job: " << Job->Start << " Time: " << Job->Time << " Right : " << Job->InitRight << std::endl;
            return;
        }
        if (Job->Start < Job->InitLeft) 
        {
            std::cout << "Problem: job starts too early" << std::endl;
            return;
        }

    }
    std::cout << "CHECK RESULT : OK" << std::endl;
            
    return;

}
