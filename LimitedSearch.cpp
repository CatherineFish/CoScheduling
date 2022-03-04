#include "LimitedSearch.h"

std::shared_ptr<PC> LimitedSearch:: MainLoop(int Mode,
                                             std::shared_ptr<Job> CurJob,
                                             std::vector<std::shared_ptr<Job>> Planned,
                                             double PPoint)
{
    std::shared_ptr<PC> Result;
    if (Mode == 0)
    {
        Result = FirstSceme(CurJob, Planned, PPoint, 1);
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

std::shared_ptr<PC> LimitedSearch:: FirstSceme(std::shared_ptr<Job> CurJob,
                                               std::vector<std::shared_ptr<Job>> Planned,
                                               System* CurSystem,
                                               int CSearch = 1)
{
    while(CSearch < SearchDepth)
    {

        auto CurPC = CurJob->ListResult.begin();
        while (CurPC != CurJob->ListResult.end())
        {
            auto LastJobOnPC = CurSystem->SystemPC->PlannedOnPC.end() - 1;
            double PPoint = LastJobOnPC->Start + LastJobOnPC->Time;
            int CheckResult = Check(CurJob, CurPC, CurSystem);
            if (!CheckResult)
            {
                if (CSearch == 1) {
                    return CurPC;
                }
                Plan(CurJob, CurPC, CurSystem, PPoint);
                Planned.push_back(CurJob);
                return CurPC; 
            }
    
        }
        auto JobForUnPlan = std::shared_ptr<Job>(Planned->end() - 1);    
        UnPlan(Planned, CurSystem);
        FirstSceme(JobForUnPlan, Planned, CurSystem, CSearch + 1);
    }
    
    std::cout << "Не удалось построить расписание" << std::endl;
    exit(1);//todo

}

void LimitedSearch::