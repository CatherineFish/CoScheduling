#include <algorithm>
#include <numeric>
#include "LimitedSearch.h"

std::shared_ptr<PC> LimitedSearch:: MainLoop(int Mode,
                                             std::shared_ptr<Job> CurJob,
                                             std::vector<std::shared_ptr<Job>> & Planned,
                                             System * CurSystem,
                                             std::vector<std::shared_ptr<Job>> & Unplanned)
{
    std::cout << "In Limited Search" << std::endl;
    std::cout << "Cur Job " << std::find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), CurJob) - CurSystem->SystemJob.begin() << std::endl;
    std::cout << "Cur Job failed PC for plan " << CurJob->ListResult.begin()->second->Num << std::endl;
    for (int i = 0; i < Planned.size(); i++)
    {
        std::cout << std::find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), Planned[i]) - CurSystem->SystemJob.begin() << " ";
    }
    std::cout << std::endl;
    
    std::shared_ptr<PC> Result;
    if (Mode == 0)
    {
        std::cout << "go" << std::endl;
    
        Result = FirstSceme(CurJob, CurJob->ListResult.begin()->second, Planned, CurSystem, Unplanned, 1, 1);
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
    //if (NewPPoint == 16) return 0;
    bool flag = false;
    double NewLeft = 0.0;
    double BSum = 0.0;
    if (CurJob->InMessage.size() != 0)
    {
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
                                               std::vector<std::shared_ptr<Job>> & Planned,
                                               System* CurSystem,
                                               std::vector<std::shared_ptr<Job>> & Unplanned,
                                               int CSearch,
                                               int Iteration)
{
    //WARNING результирующий список не содержит всех ядер, перебор не полный - не стремимся к полному
    std::cout << "Cur Depth " << CSearch << " Cur Iteration " << Iteration << std::endl;

    if (CSearch > SearchDepth || Iteration > IterationDepth)
    {
        std::cout << "Не удалось построить расписание: перебрали все на глубину поиска" << std::endl;
        if (!isBandwidthProblem) exit(1); //глобальная неудача, перебрали все на глубину поиска
        else {
            isSucces = false;
            std::cout << "here" << std::endl;
            return nullptr;
        }
    }
    
    if (Iteration != 1)
    {
        
        UpdateBList(CurSystem, CurJob);
        UpdateFList(CurSystem, CurJob);
        UpdateRList(CurJob);
    }
    std::cout << "Result List" << std::endl;

    for (const auto & CurPC: CurJob->ListResult)
    {
        std::cout << CurPC.second->Num << " ";
    }
    std::cout << std::endl;

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
    if (CurPC != CurJob->ListResult.end())
    {
        std::cout << "PC for plan " << CurPC->second->Num << std::endl;
    
    }
    auto CurPCExtra = CurJob->ListFill.begin();
    bool isEmptyResult = false;
    if (CurJob->ListResult.size() == 1)
    {
        //TODO костыль
        isEmptyResult = true;
    }
    //перебираем ядра в результирующем списке
    while ((!isEmptyResult && CurPC != CurJob->ListResult.end()) || (isEmptyResult && CurPCExtra != CurJob->ListFill.end()))
    {
        
        std::cout << "Not out " << std::endl;
        //std::cout << "Global Job Idx for Unplan: " << BadPC->PlannedOnPC[BadPC->PlannedOnPC.size() - 1] << std::endl;
        if (isEmptyResult)
            std::cout << " cur pc = " << CurPCExtra->first->Num << std::endl;
        else
            std::cout << " cur pc = " << CurPC->second->Num << std::endl;

        double NewPPoint = CurJob->InitLeft;
        
        for (const auto & SendIdx: CurJob->InMessage)
        {
            if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum == CurPC->second->ModNum)
            {
                NewPPoint = std::max(NewPPoint, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time); 
            } else if (CurSystem->SystemJob[SendIdx]->IsPlanned)
            {
                NewPPoint = std::max(NewPPoint, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time + 
                                   CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(CurSystem->SystemJob[SendIdx], CurJob)]->Dur);         
            }
        }
        std::cout << "new point " << NewPPoint << std::endl;
        
        if (!isEmptyResult && CurPC->second->PlannedOnPC.size())
        {
            int Idx = CurPC->second->PlannedOnPC[CurPC->second->PlannedOnPC.size() - 1];
            
            NewPPoint = std::max(CurSystem->SystemJob[Idx]->Start + CurSystem->SystemJob[Idx]->Time, NewPPoint);
            
        } else if (isEmptyResult && CurPCExtra->first->PlannedOnPC.size())
        {
            int Idx = CurPCExtra->first->PlannedOnPC[CurPCExtra->first->PlannedOnPC.size() - 1];
            std::cout << "Last Job on PC " << CurPCExtra->first->Num << " is " << Idx << std::endl;
            
            NewPPoint = std::max(CurSystem->SystemJob[Idx]->Start + CurSystem->SystemJob[Idx]->Time, NewPPoint);
            
        }
        std::shared_ptr<PC> NewPCForPlan;
        int CheckResult;
        if (!isEmptyResult){
            CheckResult = Check(CurJob, CurPC->second, CurSystem, NewPPoint);
            NewPCForPlan = CurPC->second;
        }
        else {
            CheckResult = Check(CurJob, CurPCExtra->first, CurSystem, NewPPoint);
            NewPCForPlan = CurPCExtra->first;
        }
            
        
        std::cout << "CheckResult " << CheckResult << std::endl;
        
        if (!CheckResult && NewPCForPlan != BadPC)
        {
            std::cout << "GOOD RES" << std::endl;
            CurSystem->PPoint = NewPPoint;
            if (!isEmptyResult)
            {
                return std::shared_ptr<PC>(CurPC->second); 
            }
            else { 
                return std::shared_ptr<PC>(CurPCExtra->first);
            }
        }
        if (isEmptyResult)
        {
            CurPCExtra++;
            std::cout << "Extra "  << std::endl;
            
            std::cout << "Extra " << (CurPCExtra != CurJob->ListFill.end()) << std::endl;
            
        } else {
            CurPC++;
        }
        

    }
    std::cout << "List Result ended " << std::endl;

    //перебрали весь результирующий список для текущей работы
    if (Planned.size() == 0)
    {
        std::cout << "Не удалось построить расписание: перебрали все имеющиеся запланированные работы" << std::endl;
        exit(1);

        // пока что глобальный выход
        /*
        if (!isBandwidthProblem) exit(1); //глобальная неудача, перебрали все имеющиеся запланированные работы
        else {
            isSucces = false;
            std::cout << "here ?" << std::endl;
            return nullptr;
        }*/

    }
    std::cout << "Want to unplan" << std::endl;
    
    auto JobForUnPlan = std::shared_ptr<Job>(Planned[Planned.size() - 1]);
    std::cout << "Job for unplan " <<  std::find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), JobForUnPlan) - CurSystem->SystemJob.begin() << std::endl;
    
    auto PCForUnPlan = JobForUnPlan->JobPC;
    UnPlan(Planned, CurSystem, Unplanned);
    auto NewPC = FirstSceme(JobForUnPlan, PCForUnPlan, Planned, CurSystem, Unplanned, CSearch + 1, Iteration);
    std::cout << (NewPC == nullptr) << std::endl;
    std::cout << "We have new PC " << NewPC->Num << std::endl;
    std::cout << NewPC->PlannedOnPC.size() << std::endl;
    Plan(JobForUnPlan, NewPC, Planned, CurSystem, Unplanned);

    return FirstSceme(CurJob, nullptr, Planned, CurSystem, Unplanned, CSearch, Iteration + 1);
}


void LimitedSearch:: Plan (std::shared_ptr<Job> CurJob,
                           std::shared_ptr<PC> CurPC, 
                           std::vector<std::shared_ptr<Job>> & Planned,
                           System * CurSystem,
                           std::vector<std::shared_ptr<Job>> & Unplanned)
{
    if (CurPC->PlannedOnPC.size())
    {
        int Idx = CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1];
        CurJob->NewLimitForPlan[CurPC->Num] = std::max(CurSystem->SystemJob[Idx]->Start + CurSystem->SystemJob[Idx]->Time, CurJob->NewLimitForPlan[CurPC->Num]);
    }
    

    //Планируем
    CurJob->JobPC = std::shared_ptr<PC>(CurPC);
    CurJob->Start = CurJob->NewLimitForPlan[CurPC->Num];
    CurJob->JobPC->PC_PPoint = CurJob->NewLimitForPlan[CurPC->Num] + CurJob->Time;
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
            //auto CurMes = CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1];
            CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Size = CurJob->CMessageSize;
            CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Dur = CurJob->Left -
                          CurJob->PreviousJob->Start -
                          CurJob->PreviousJob->Time;            
            CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Bandwidth = CurJob->CMessageSize / 
                                (CurJob->Left -
                                CurJob->PreviousJob->Start - 
                                CurJob->PreviousJob->Time);
            
            CurSystem->CurBLackCoef.Update(false, CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Bandwidth, CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->StabilityCoef.CountNotPlanned, 
                                           CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->StabilityCoef.CountInPeriod);
            
            CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                  std::shared_ptr<Job>>
                                  (CurJob->PreviousJob, CurJob)] = CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1];
                
        }
        //чтобы не перегружать CurSystem->JobMessage, там только одно контекстное сообщение 
                      
    }
    
    //обновление коэффициентов для сообщений
    if (CurJob->InMessage.size() != 0)
    {
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
    }
    
    //обновляем ядро, на которое поставили работу
    int add = std::find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), CurJob) - CurSystem->SystemJob.begin();
    CurJob->JobPC->PlannedOnPC.push_back(add);
    
    //обновляем список запланированных работ
    Planned.push_back(std::shared_ptr<Job>(CurJob));
    
    //обновляем точку планирования
    CurSystem->PPoint = UpdatePPoint(CurSystem, Unplanned);  

}


double LimitedSearch:: UpdatePPoint(System* CurSystem, std::vector<std::shared_ptr<Job>> & Unplanned)
{
    double NewPoint = CurSystem->LCMPeriod, NewPoint2 = CurSystem->LCMPeriod;
    for (const auto & CurJob: Unplanned)
    {
        NewPoint = std::min(NewPoint, CurJob->Left);
    }
    
    for (const auto & CurPC: CurSystem->SystemPC)
    {
        if (CurPC->PlannedOnPC.size() == 0) {
            NewPoint2 = 0.0;
            break;
        }
    
        NewPoint2 = std::min(NewPoint2, CurSystem->SystemJob[CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1]]->Start + 
                            CurSystem->SystemJob[CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1]]->Time);
    }
    
    NewPoint = std::max(NewPoint, NewPoint2);
    
    /*
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
    
void LimitedSearch:: UnPlan (std::vector<std::shared_ptr<Job>> & Planned,
                             System * CurSystem,
                             std::vector<std::shared_ptr<Job>> & Unplanned)
{
    
    int size = Planned.size() - 1;
    
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
                    auto noElem = std::remove(CurSystem->SystemCMessage.begin(), CurSystem->SystemCMessage.end(), CurMes);
                    CurSystem->SystemCMessage.erase(noElem, CurSystem->SystemCMessage.end());
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
    
    CurSystem->PPoint = UpdatePPoint(CurSystem, Unplanned); 

    //delete Planned[size]->JobPC;//тут могут быть проблемы
    
    Planned[size]->IsPlanned = false;
    Planned.pop_back();
    
    
    return;
}  


void LimitedSearch:: UpdateFList (System * CurSystem, std::shared_ptr<Job> CurJob)
{
    std::vector<int> NewLimit;
    CurJob->NewLimitForPlan.clear();
    CurJob->isUpdated.clear();
        
    for (size_t k = 0; k < CurSystem->SystemPC.size(); k++)
    {
        //пересчитываем новый левый директивный интервал работы для конкретного ядра
        //учитываем, что надо ждать сообщения
        bool flag = false;
        double NewLeft = 0.0;
        for (const auto & SendIdx: CurJob->InMessage)
        {
            if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum == CurJob->ListResult.begin()->second->ModNum)
            {
                flag = true;
                NewLeft = std::max(NewLeft, CurSystem->SystemJob[SendIdx]->Start + CurSystem->SystemJob[SendIdx]->Time); 
            } else if (CurSystem->SystemJob[SendIdx]->IsPlanned)
            {
                flag = true; 
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
        CurJob->NewLimitForPlan.push_back(NewLeft);
        CurJob->isUpdated.push_back(flag);
            
        if (flag)
        {
            CurJob->ListFill[std::shared_ptr<PC>(CurSystem->SystemPC[k])] = CurSystem->SystemPC[k]->PC_PPoint <= std::max(NewLeft, CurJob->InitLeft) ? 
                                                                                  0.0 : CurSystem->SystemPC[k]->PC_PPoint;   
        } else 
        {
            //если ядро освобождается раньше того, как можно запланировать работу, но ставим 0
            CurJob->ListFill[std::shared_ptr<PC>(CurSystem->SystemPC[k])] = CurSystem->SystemPC[k]->PC_PPoint <= CurJob->Left ? 
                                                                                  0.0 : CurSystem->SystemPC[k]->PC_PPoint;   
        }
        NewLimit.emplace_back(CurJob->ListFill[std::shared_ptr<PC>(CurSystem->SystemPC[k])]);
    }


    auto PCPointsSum = std::accumulate(NewLimit.begin(), NewLimit.end(), decltype(NewLimit)::value_type(0));   
    LimitForPC = PCPointsSum != 0.0 ? PCPointsSum / NewLimit.size() : PCPointsSum;
    return;
}


void LimitedSearch:: UpdateBList (System* CurSystem, std::shared_ptr<Job> CurJob)
{
    double CurScore;
    bool IsContextMes;
    
    for (size_t k = 0; k < CurSystem->SystemPC.size(); k++)
    {
        CurScore = 0.0;
        for (const auto & SendIdx: CurJob->InMessage)
        {
            if (CurSystem->SystemJob[SendIdx]->IsPlanned && CurSystem->SystemJob[SendIdx]->JobPC->ModNum != CurSystem->SystemPC[k]->ModNum)
            {
                CurScore += CurJob->Period * 
                            CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>
                                                  (CurSystem->SystemJob[SendIdx], CurJob)]->Size *
                            CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>
                                                  (CurSystem->SystemJob[SendIdx], CurJob)]->StabilityCoef.Value;
                            ; //внести коэффициент
            }
        }
        if (CurJob->PreviousJob && CurJob->PreviousJob->IsPlanned && CurJob->PreviousJob->JobPC->ModNum != CurSystem->SystemPC[k]->ModNum)
        {
            IsContextMes = false;
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
                //если сообщения нет, добавляем пропускную способность в счет ядра
                CurScore += CurJob->Period * CurJob->CMessageSize * CurSystem->CurBLackCoef.Value;     
            }
            
        }
            
        CurJob->ListBandwidth[std::shared_ptr<PC>(CurSystem->SystemPC[k])] = CurScore;   
    }
    return;
}


void LimitedSearch:: UpdateRList (std::shared_ptr<Job> CurJob)
{
    if (RListMood == 1)
    {
        //сначала по ядрам, потом по пропускной способности
        CurJob->ListResult.clear();
        for (std::pair<std::shared_ptr<PC>, double> CurPC : CurJob->ListFill) 
        {
            if (CurPC.second <= LimitForPC)
            {
                CurJob->ListResult.insert(decltype(CurJob->ListResult)::value_type(CurJob->ListBandwidth[CurPC.first], CurPC.first));
            }
        }
    } else if (RListMood == 2)
    {
        // сначала по пропускной способности, потом по ядрам
        CurJob->ListResult.clear();
        for (std::pair<std::shared_ptr<PC>, double> CurPC : CurJob->ListBandwidth) 
        {
            if (CurPC.second <= CritLimit)
            {
                CurJob->ListResult.insert(decltype(CurJob->ListResult)::value_type(CurJob->ListFill[CurPC.first], CurPC.first));
            }
            
        }
    } else 
    {
        std::cout << "Invalid Mood for Result List" << std::endl;
        exit(1);
    }
    return;
}
