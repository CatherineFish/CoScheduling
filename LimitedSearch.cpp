#include <algorithm>
#include "LimitedSearch.h"

std::shared_ptr<PC> LimitedSearch:: MainLoop(int Mode,
                                             std::shared_ptr<Job> CurJob,
                                             std::vector<std::shared_ptr<Job>> Planned,
                                             System * CurSystem)
{
    std::shared_ptr<PC> Result;
    if (Mode == 0)
    {
        Result = FirstSceme(CurJob, CurJob->ListResult.begin()->second, Planned, CurSystem, 1, 1);
    } else if (Mode == 1)
    {

    } else if (Mode == 2)
    {

    } else 
    {
        std::cout << "Invalid mode" << std::endl;
    }
    return Result;
}


int LimitedSearch:: Check(std::shared_ptr<Job> CurJob,
                          std::shared_ptr<PC> PCForPlan,
                          System* CurSystem,
                          double NewPPoint)
{
    bool flag = false;
    double NewLeft = 0.0;
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
        } else if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum == PCForPlan->ModNum)
        {
            //паралельно одновляем левый директивный срок
            flag = true;
            NewLeft = std::max(NewLeft, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time); 
        } else if (CurSystem->SystemJob[SendIdx]->IsPlanned)
        {
            flag = true; 
            NewLeft = std::max(NewLeft, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time + 
                               CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->Dur); 
            
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
                NewLeft = std::max(NewLeft, CurJob->PreviousJob->Start + CurJob->PreviousJob->Time + CurMes->Dur);
                break;
            }
        }
        if (!IsContextMes)
        {
            BSum += CurJob->CMessageSize / (CurJob->Left - CurJob->PreviousJob->Start - CurJob->PreviousJob->Time);
        }
        if (flag)
        {
            CurJob->Left = std::max(NewLeft, CurJob->InitLeft);    
        }
        CurJob->Left = std::max(PCForPlan->PC_PPoint, CurJob->Left);
                
    }
    if (CurSystem->BTotal <= BSum + CurSystem->CurBand)
    {
        return 1; // нехватка пропускной способности
    }
    if (CurJob->Right < NewPPoint + CurJob->Time)
    {
        return 2; //нарушение директивных сроков
    }
    return 0;
}

std::shared_ptr<PC> LimitedSearch:: FirstSceme(std::shared_ptr<Job> CurJob,
                                               std::shared_ptr<PC> BadPC, 
                                               std::vector<std::shared_ptr<Job>> Planned,
                                               System* CurSystem,
                                               int CSearch,
                                               int Iteration)
{
    //TODO результирующий список не содержит всех ядер, перебор не полный
    if (CSearch > SearchDepth || Iteration > IterationDepth)
    {
        std::cout << "Не удалось построить расписание: перебрали все на глубину поиска" << std::endl;
        exit(1); //глобальная неудача, перебрали все на глубину поиска
    }

    auto CurPC = CurJob->ListResult.begin();     
    if (BadPC != nullptr) 
    {
        while (CurPC != CurJob->ListResult.end())
        {
            if (CurPC->second == BadPC)
            {
                CurPC++;
                break;
            }
            CurPC++;
        }
    }
    //перебираем ядра в результирующем списке
    while (CurPC != CurJob->ListResult.end())
    {
        int Idx = *(CurPC->second->PlannedOnPC.rend());
        double NewPPoint = std::max(CurSystem->SystemJob[Idx]->Start + CurSystem->SystemJob[Idx]->Time, CurJob->InitLeft);
        int CheckResult = Check(CurJob, CurPC->second, CurSystem, NewPPoint);
        if (!CheckResult)
        {
            return CurPC->second; 
        }

    }
    //перебрали весь результирующий список для текущей работы
    if (Planned.size() == 0)
    {
        std::cout << "Не удалось построить расписание: перебрали все имеющиеся запланированные работы" << std::endl;
        exit(1); //глобальная неудача, перебрали все имеющиеся запланированные работы
    }
    auto JobForUnPlan = std::shared_ptr<Job>(*Planned.rend());
    auto PCForUnPlan = JobForUnPlan->JobPC;    
    UnPlan(Planned, CurSystem);
    auto NewPC = FirstSceme(JobForUnPlan, PCForUnPlan, Planned, CurSystem, CSearch + 1, Iteration);
    Plan(JobForUnPlan, NewPC, Planned, CurSystem);
    return FirstSceme(CurJob, nullptr, Planned, CurSystem, CSearch - 1, Iteration + 1);
}


void LimitedSearch:: Plan (std::shared_ptr<Job> CurJob,
                           std::shared_ptr<PC> CurPC, 
                           std::vector<std::shared_ptr<Job>> Planned,
                           System * CurSystem)
{
    int Idx = *(CurPC->PlannedOnPC.rend());
    double NewPPoint = std::max(CurSystem->SystemJob[Idx]->Start + CurSystem->SystemJob[Idx]->Time, CurJob->InitLeft);
    
    //Планируем
    CurJob->JobPC = std::shared_ptr<PC>(CurPC);
    CurJob->Start = NewPPoint;
    CurJob->JobPC->PC_PPoint = NewPPoint + CurJob->Time;
    CurJob->IsPlanned = true;

    //Если предыдущий экземпляр работы запланирован на дургой модуль, формируем контекстное сообщение, если его нет
    if (CurJob->PreviousJob && CurJob->PreviousJob->IsPlanned && 
        CurJob->JobPC->ModNum != CurJob->PreviousJob->JobPC->ModNum)
    {
        bool IsContextMes = false;
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
            //контекстного сообщения нет - создаём
            CurSystem->SystemCMessage.push_back(std::make_shared<ContextMessage>(CurJob->PreviousJob, CurJob));
            auto CurMes = CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1];
            CurMes->Size = CurJob->CMessageSize;
            CurMes->Dur = CurJob->Left -
                          CurJob->PreviousJob->Start -
                          CurJob->PreviousJob->Time;            
            CurMes->Bandwidth = CurJob->CMessageSize / 
                                (CurJob->Left -
                                CurJob->PreviousJob->Start - 
                                CurJob->PreviousJob->Time);
            CurSystem->CurBLackCoef.Update(false, CurMes->Bandwidth, CurMes->StabilityCoef.CountNotPlanned, CurMes->StabilityCoef.CountInPeriod);
            
            CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                  std::shared_ptr<Job>>
                                  (CurJob->PreviousJob, CurJob)] = CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1];
                
        }
        //чтобы не перегружать CurSystem->JobMessage, там только одно контекстное сообщение 
                      
    }


    //обновление коэффициентов для сообщений
    for (const auto & SendIdx: CurJob->InMessage)
    {
        std::shared_ptr<Message> CurMes = CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                                                std::shared_ptr<Job>>
                                                                (CurSystem->SystemJob[SendIdx],
                                                                CurJob)];

        if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum != CurJob->JobPC->ModNum)
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
    
    //обновляем ядро, на которое поставили работу
    CurJob->JobPC->PlannedOnPC.emplace_back(find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), CurJob) - CurSystem->SystemJob.begin());
    
    //обновляем список запланированных работ
    Planned.push_back(std::shared_ptr<Job>(CurJob));
    
    //обновляем точку планирования
    CurSystem->PPoint = UpdatePPoint(CurSystem);        
}


double LimitedSearch:: UpdatePPoint(System* CurSystem)
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
    
void LimitedSearch:: UnPlan (std::vector<std::shared_ptr<Job>> Planned,
                             System * CurSystem)
{
    size_t size = Planned.size() - 1;
    

    //Если предыдущий экземпляр работы запланирован на дургой модуль, есть контекстное сообщение, убираем его
    if (Planned[size]->PreviousJob && Planned[size]->PreviousJob->IsPlanned && 
        Planned[size]->JobPC->ModNum != Planned[size]->PreviousJob->JobPC->ModNum)
    {
        //проверяем, что контекстное сообщение создано только из-за этой работы, иначе не удаляем сообщение
        for (const auto & CurMes: CurSystem->SystemCMessage)
        {
            if (CurMes->Src->NumOfTask == Planned[size]->NumOfTask)
            {
                if (CurMes->Dest == Planned[size])
                {
                    //контекстное из-за рассматриваемой работы - удаляем
                    CurSystem->JobMessage.erase(std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>
                                                (Planned[size]->PreviousJob, Planned[size]));
                    std::remove(CurSystem->SystemCMessage.begin(), CurSystem->SystemCMessage.end(), CurMes);
                }
                break;
            }
        }             
    }


    //обновление коэффициентов для сообщений
    for (const auto & SendIdx: Planned[size]->InMessage)
    {
        std::shared_ptr<Message> CurMes = CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                                                std::shared_ptr<Job>>
                                                                (CurSystem->SystemJob[SendIdx],
                                                                Planned[size])];

        if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum != Planned[size]->JobPC->ModNum)
        {
            if (Planned[size]->Num == CurMes->StabilityCoef.NumOfPlanned) {
                CurMes->StabilityCoef.Reload(true);    
            }
            
            CurSystem->CurBLackCoef.Reload(true, CurMes->Bandwidth, CurMes->StabilityCoef.CountNotPlanned, CurMes->StabilityCoef.CountInPeriod);
        } else {
            CurMes->StabilityCoef.Reload(false);
            CurSystem->CurBLackCoef.Reload(false, CurMes->Bandwidth, CurMes->StabilityCoef.CountNotPlanned, CurMes->StabilityCoef.CountInPeriod);
        }
    }
    
    //обновляем ядро, на которое поставили работу, она там точно последняя
    Planned[size]->JobPC->PlannedOnPC.pop_back();
    Planned[size]->JobPC->PC_PPoint = CurSystem->SystemJob[*(Planned[size]->JobPC->PlannedOnPC.rend())]->Start + CurSystem->SystemJob[*(Planned[size]->JobPC->PlannedOnPC.rend())]->Time;
    
    //обновляем список запланированных работ
    
    //обновляем точку планирования
    CurSystem->PPoint = UpdatePPoint(CurSystem);    
    //delete Planned[size]->JobPC;//тут могут быть проблемы
    Planned[size]->IsPlanned = false;
    Planned.pop_back();
    return;
}  

