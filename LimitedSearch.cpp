#include <algorithm>
#include <numeric>
#include "LimitedSearch.h"

std::shared_ptr<PC> LimitedSearch:: MainLoop(int Mode,
                                             std::shared_ptr<Job> CurJob,
                                             std::vector<std::shared_ptr<Job>> & Planned,
                                             System * CurSystem,
                                             std::vector<std::shared_ptr<Job>> & Unplanned)
{
    //std::cout << "In Limited Search" << std::endl;
    //std::cout << "Cur Job " << CurJob->Time << std::endl;
    //std::cout << "Cur Job failed PC for plan " << CurJob->ListResult.begin()->second->Num << std::endl;
    for (int i = 0; i < Planned.size(); i++)
    {
        //std::cout << std::find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), Planned[i]) - CurSystem->SystemJob.begin() << " ";
    }
    //std::cout << std::endl;
    
    std::shared_ptr<PC> Result;
    if (Mode == 0)
    {
        //std::cout << "go" << std::endl;
    
        Result = FirstSceme(CurJob, CurJob->ListResult.begin()->second, Planned, CurSystem, Unplanned, 1, 1);
    } else if (Mode == 1)
    {

    } else if (Mode == 2)
    {

    } else 
    {
        //std::cout << "Invalid mode" << std::endl;
    }
    return Result;
}


int LimitedSearch:: Check(std::shared_ptr<Job> CurJob,
                          std::shared_ptr<PC> PCForPlan,
                          System* CurSystem,
                          double NewPPoint)
{
    //if (NewPPoint == 16) return 0;
    //std::cout << "Check Start" << std::endl;
    bool flag = false;
    double NewLeft = 0.0;
    double BSum = 0.0;
    //std::cout << CurJob->InMessage.size() << std::endl;
    if (CurJob->InMessage.size() != 0)
    {
        for (const auto & SendIdx: CurJob->InMessage)
        {
            //std::cout << SendIdx << " " << CurSystem->SystemJob.size() << std::endl;
            //std::cout << "Chech sender " << CurSystem->SystemJob[SendIdx] << std::endl;
        
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
    //std::cout << "Here" << std::endl;
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
    //std::cout << "Here Too" << std::endl;
    
    if (CurSystem->BTotal <= BSum + CurSystem->CurBand)
    {
        return 1; // нехватка пропускной способности
    }
    if (CurJob->Right < NewPPoint + CurJob->Time)
    {
        return 2; //нарушение директивных сроков
    }
    //std::cout << "Correct Res" << std::endl;
    
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
    //std::cout << "Cur Depth " << CSearch << " Cur Iteration " << Iteration << std::endl;

    if (CSearch > SearchDepth || Iteration > IterationDepth)
    {
        //std::cout << "Не удалось построить расписание: перебрали все на глубину поиска" << std::endl;
        if (!isBandwidthProblem) {
            std::cout << 0 << std::endl;
            exit(0); //глобальная неудача, перебрали все на глубину поиска
        }
        else {
            isSucces = false;
            //std::cout << "here" << std::endl;
            return nullptr;
        }
    }
    
    if (Iteration != 1)
    {
        //std::cout << "Size before " << CurJob->ListResult.size() << std::endl;
        for (int i = 0; i < CurSystem->SystemCMessage.size(); i++)
        {
            //std::cout << CurSystem->SystemCMessage[i]->Dur << std::endl;
        }
        UpdateBList(CurSystem, CurJob);
        //std::cout << "Update_1" << std::endl;
        UpdateFList(CurSystem, CurJob);
        //std::cout << "Update_2" << std::endl;
        
        UpdateRList(CurJob);
        //std::cout << "Size after " << CurJob->ListResult.size() << std::endl;
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
    if (CurPC != CurJob->ListResult.end())
    {
        //std::cout << "PC for plan " << CurPC->second->Num << std::endl;
    
    }
    auto CurPCExtra = CurJob->ListFill.begin();
    bool isEmptyResult = false;
    if (CurJob->ListResult.size() == 1)
    {
        //TODO костыль
        isEmptyResult = true;
        //std::cout << "CHECK LIST RESULT " << (CurPC != CurJob->ListResult.end()) << " " << (CurPCExtra != CurJob->ListFill.end()) << std::endl;  
    }
    //перебираем ядра в результирующем списке
    while (CurPC != CurJob->ListResult.end() || (isEmptyResult && CurPCExtra != CurJob->ListFill.end()))
    {
        //std::cout << "in cycle" << std::endl;
        //std::cout << CurJob->Time << std::endl;
        
        ////std::cout << "Global Job Idx for Unplan: " << BadPC->PlannedOnPC[BadPC->PlannedOnPC.size() - 1] << std::endl;
        double NewPPoint = CurJob->InitLeft;
        //std::cout << "new point " << NewPPoint << std::endl;
        
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

        if (!isEmptyResult && CurPC->second->PlannedOnPC.size())
        {
            int Idx = CurPC->second->PlannedOnPC[CurPC->second->PlannedOnPC.size() - 1];
            
            NewPPoint = std::max(CurSystem->SystemJob[Idx]->Start + CurSystem->SystemJob[Idx]->Time, NewPPoint);
            
        } else if (isEmptyResult && CurPCExtra->first->PlannedOnPC.size())
        {
            int Idx = CurPCExtra->first->PlannedOnPC[CurPCExtra->first->PlannedOnPC.size() - 1];
            
            NewPPoint = std::max(CurSystem->SystemJob[Idx]->Start + CurSystem->SystemJob[Idx]->Time, NewPPoint);
            
        }
        //std::cout << "! new point " << NewPPoint << std::endl;
        
        int CheckResult;
        if (!isEmptyResult)
            CheckResult = Check(CurJob, CurPC->second, CurSystem, NewPPoint);
        else 
            CheckResult = Check(CurJob, CurPCExtra->first, CurSystem, NewPPoint);
            
        
        //std::cout << "CheckResult " << CheckResult << std::endl;
        
        if (!CheckResult)
        {
            CurSystem->PPoint = NewPPoint;
            //std::cout << "I RETURN" << std::endl;

            if (!isEmptyResult)
            {
                //std::cout << CurPC->second->PlannedOnPC.size() << std::endl;
            
                return std::shared_ptr<PC>(CurPC->second); 
            }
            else { 
                //std::cout << CurPCExtra->first->PlannedOnPC.size() << std::endl;
            
                return std::shared_ptr<PC>(CurPCExtra->first);
            }
        }
        if (isEmptyResult)
        {
            CurPCExtra++;
        } else {
            CurPC++;
        }
        

    }
    //std::cout << "List Result Size : " << CurJob->ListResult.size() << std::endl;
    //std::cout << "List Result ended " << std::endl;

    //перебрали весь результирующий список для текущей работы
    if (Planned.size() == 0)
    {
        
        std::cout << 0 << std::endl;
        exit(0);
        /*
        //std::cout << "Не удалось построить расписание: перебрали все имеющиеся запланированные работы" << std::endl;
        if (!isBandwidthProblem) {
            //exit(0); //глобальная неудача, перебрали все имеющиеся запланированные работы
            std::cout << 0 << std::endl;
            exit(0);
        }
        else {
            isSucces = false;
            //std::cout << "here ?" << std::endl;
            return nullptr;
        }
        */
    }
    //std::cout << "Want to unplan" << std::endl;
    
    auto JobForUnPlan = std::shared_ptr<Job>(Planned[Planned.size() - 1]);
    //std::cout << "Job for unplan " << JobForUnPlan->Time << std::endl;
    
    auto PCForUnPlan = JobForUnPlan->JobPC;
    //std::cout << "GLOB Before " << Planned.size() << std::endl;     
    
    UnPlan(Planned, CurSystem, Unplanned);
    
    //std::cout << "GLOB After " << Planned.size() << std::endl;
         
    auto NewPC = FirstSceme(JobForUnPlan, PCForUnPlan, Planned, CurSystem, Unplanned, CSearch + 1, Iteration);
    //std::cout << (NewPC == nullptr) << std::endl;
    //std::cout << "We have new PC " << NewPC->Num << std::endl;
    //std::cout << NewPC->PlannedOnPC.size() << std::endl;
    for (int i = 0; i < CurSystem->SystemPC.size(); i++)
    {
        //std::cout << CurSystem->SystemPC[i]->PlannedOnPC.size() << std::endl;    
        
    }
    Plan(JobForUnPlan, NewPC, Planned, CurSystem, Unplanned);

    //std::cout << "Plan ended" << std::endl;
    return FirstSceme(CurJob, nullptr, Planned, CurSystem, Unplanned, CSearch, Iteration + 1);
}


void LimitedSearch:: Plan (std::shared_ptr<Job> CurJob,
                           std::shared_ptr<PC> CurPC, 
                           std::vector<std::shared_ptr<Job>> & Planned,
                           System * CurSystem,
                           std::vector<std::shared_ptr<Job>> & Unplanned)
{
    //std::cout << "Plan_1" << std::endl;
    if (CurPC->PlannedOnPC.size())
    {
        int Idx = CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1];
        CurJob->NewLimitForPlan[CurPC->Num] = std::max(CurSystem->SystemJob[Idx]->Start + CurSystem->SystemJob[Idx]->Time, CurJob->NewLimitForPlan[CurPC->Num]);
    }
    

    //std::cout << "Plan_2" << std::endl;
    //std::cout << CurPC->PlannedOnPC.size() << std::endl;
    //Планируем
    CurJob->JobPC = std::shared_ptr<PC>(CurPC);
    CurJob->Start = CurJob->NewLimitForPlan[CurPC->Num];
    CurJob->JobPC->PC_PPoint = CurJob->NewLimitForPlan[CurPC->Num] + CurJob->Time;
    CurJob->IsPlanned = true;
    //std::cout << "Plan_3" << std::endl;
    
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
            //std::cout << "UPDATE MESSAGES!!!!" << std::endl;
            //std::cout << CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Dur << std::endl;
            
            CurSystem->CurBLackCoef.Update(false, CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Bandwidth, CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->StabilityCoef.CountNotPlanned, 
                                           CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->StabilityCoef.CountInPeriod);
            
            CurSystem->JobMessage[std::pair<std::shared_ptr<Job>, 
                                  std::shared_ptr<Job>>
                                  (CurJob->PreviousJob, CurJob)] = CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1];
                
        }
        //чтобы не перегружать CurSystem->JobMessage, там только одно контекстное сообщение 
                      
    }
    //std::cout << "Plan_4" << std::endl;
    

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
    //std::cout << "Plan_5" << std::endl;
    
    //обновляем ядро, на которое поставили работу
    //std::cout << CurJob->Time << std::endl;
    //std::cout << CurJob->JobPC->Num << std::endl;
    //std::cout << CurJob->JobPC->PlannedOnPC.size() << std::endl;
    int add = std::find(CurSystem->SystemJob.begin(), CurSystem->SystemJob.end(), CurJob) - CurSystem->SystemJob.begin();
    //std::cout << add << std::endl;
    CurJob->JobPC->PlannedOnPC.push_back(add);
    //std::cout << "Plan_6" << std::endl;
    
    //обновляем список запланированных работ
    Planned.push_back(std::shared_ptr<Job>(CurJob));
    
    //обновляем точку планирования
    CurSystem->PPoint = UpdatePPoint(CurSystem, Unplanned);  

}


double LimitedSearch:: UpdatePPoint(System* CurSystem, std::vector<std::shared_ptr<Job>> & Unplanned)
{
    //std::cout << "Update PPoint_1" << std::endl;
    double NewPoint = CurSystem->LCMPeriod, NewPoint2 = CurSystem->LCMPeriod;
    for (const auto & CurJob: Unplanned)
    {
        NewPoint = std::min(NewPoint, CurJob->Left);
    }
    //std::cout << "Update PPoint_2" << std::endl;
    
    for (const auto & CurPC: CurSystem->SystemPC)
    {
        if (CurPC->PlannedOnPC.size() == 0) {
            NewPoint2 = 0.0;
            break;
        }
        //std::cout << "pp" << std::endl;
        for (int ii = 0; ii < CurPC->PlannedOnPC.size(); ii++)
        {
            //std::cout << CurPC->PlannedOnPC[ii] << " ";
        }
        //std::cout << std::endl;
        //std::cout << CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1] << std::endl;
        //std::cout << CurSystem->SystemJob.size() << std::endl;
        for (int i = 0; i < CurSystem->SystemJob.size(); i++)
        {
            //std::cout << "I = " << i << "  elem = " << CurSystem->SystemJob[i]->Time << std::endl;
        }
        //std::cout << CurSystem->SystemJob[CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1]]->Start << std::endl;
        //std::cout << "not end" << std::endl;
        NewPoint2 = std::min(NewPoint2, CurSystem->SystemJob[CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1]]->Start + 
                            CurSystem->SystemJob[CurPC->PlannedOnPC[CurPC->PlannedOnPC.size() - 1]]->Time);
    }
    //std::cout << "Update PPoint_3" << std::endl;
    
    NewPoint = std::max(NewPoint, NewPoint2);
    //std::cout << "Update PPoint_4" << std::endl;
    
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
    //std::cout << "Size after : " << size << std::endl;

    //Если предыдущий экземпляр работы запланирован на дургой модуль, есть контекстное сообщение, убираем его
    if (Planned[size]->PreviousJob && Planned[size]->PreviousJob->IsPlanned && 
        Planned[size]->JobPC->ModNum != Planned[size]->PreviousJob->JobPC->ModNum)
    {
        //std::cout << "INOUT" << std::endl;
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
                    //std::cout << "Size befor" << CurSystem->SystemCMessage.size() << std::endl; 
                    auto noElem = std::remove(CurSystem->SystemCMessage.begin(), CurSystem->SystemCMessage.end(), CurMes);
                    CurSystem->SystemCMessage.erase(noElem, CurSystem->SystemCMessage.end());
                    //std::cout << "Size befor" << CurSystem->SystemCMessage.size() << std::endl;
                }
                break;
            }
        }
        
                     
    }

    
        
    //std::cout << "UnPlan_1" << std::endl;

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
    //std::cout << "UnPlan_2" << std::endl;
    
    //обновляем ядро, на которое поставили работу, она там точно последняя
    Planned[size]->JobPC->PlannedOnPC.pop_back();
    Planned[size]->JobPC->PC_PPoint = CurSystem->SystemJob[*(Planned[size]->JobPC->PlannedOnPC.rend())]->Start + CurSystem->SystemJob[*(Planned[size]->JobPC->PlannedOnPC.rend())]->Time;
    
    //обновляем список запланированных работ
    //std::cout << "UnPlan_3" << std::endl;
    
    //обновляем точку планирования
    
    CurSystem->PPoint = UpdatePPoint(CurSystem, Unplanned); 

    //delete Planned[size]->JobPC;//тут могут быть проблемы
    //std::cout << "UnPlan_4" << std::endl;
    
    Planned[size]->IsPlanned = false;
    //std::cout << "UnPlan_5" << std::endl;
    //std::cout << "Before Size " << Planned.size() << std::endl;
    Planned.pop_back();
    
    //std::cout << "New Size " << Planned.size() << std::endl;
    //std::cout << "Unplanned" << std::endl;

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
    //std::cout << "Update BList" << std::endl;
    for (int i = 0; i < CurSystem->SystemCMessage.size(); i++)
    {
        //std::cout << CurSystem->SystemCMessage[i]->Dur << std::endl;
    }
    //std::cout << "Update BList 2" << std::endl;
    
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
            //std::cout << "Here" << std::endl;
            //std::cout << CurSystem->SystemCMessage.size() << std::endl;
            for (int i = 0; i < CurSystem->SystemCMessage.size(); i++)
            {
                //std::cout << CurSystem->SystemCMessage[i]->Dur << std::endl;
            }
            ////std::cout << CurSystem->SystemCMessage[CurSystem->SystemCMessage.size() - 1]->Dur << std::endl;
            for (const auto & CurMes: CurSystem->SystemCMessage)
            {
                //std::cout << "In" << std::endl;
                //std::cout << CurMes->Dur << std::endl;
                //std::cout << CurMes->Src->NumOfTask << std::endl;
                //std::cout << "first" << std::endl;
                //std::cout << CurJob->NumOfTask << std::endl;
                //std::cout << "Test failed" << std::endl;
            

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
        //std::cout << "Not Here" << std::endl;
            
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
        //std::cout << "Invalid Mood for Result List" << std::endl;
        exit(0);
    }
    return;
}
