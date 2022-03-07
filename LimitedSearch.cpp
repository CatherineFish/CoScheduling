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
    //UnPlan(Planned, CurSystem);
    auto NewPC = FirstSceme(JobForUnPlan, PCForUnPlan, Planned, CurSystem, CSearch + 1, Iteration);
    //Plan(JobForUnPlan, NewPC);
    return FirstSceme(CurJob, nullptr, Planned, CurSystem, CSearch - 1, Iteration + 1);
}
    
    

