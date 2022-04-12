#pragma once 

#include <vector>
#include <iostream>
#include <memory>

#include "MainClasses.h"
#include "CoefClasses.h"


class LimitedSearch
{
public:
    int RListMood; // режим формирования итогового списка, берется из основного алгоритма
    int SearchDepth; // Experiment
    int IterationDepth; // Experiment
    bool isBandwidthProblem; // флаг запуска при недостатке пропускной способности сети
    bool isSucces = true; // флаг статуса завершения перебора при недостатке пропускной способности сети
    
    LimitedSearch(int RListMood_,
                  int Depth = 10, 
                  int Iteration = 2, 
                  bool isBand=false) : RListMood(RListMood_),
                                       SearchDepth(Depth), 
                                       IterationDepth(Iteration),
                                       isBandwidthProblem(isBand) {}
    ~LimitedSearch() = default;
    std::shared_ptr<PC> MainLoop(int Mode,
                                 std::shared_ptr<Job> CurJob,
                                 std::vector<std::shared_ptr<Job>> Planned,
                                 System * CurSystem,
                                 std::vector<std::shared_ptr<Job>> Unplanned);
    std::shared_ptr<PC> FirstSceme(std::shared_ptr<Job> CurJob,
                                   std::shared_ptr<PC> BadPC, 
                                   std::vector<std::shared_ptr<Job>> Planned,
                                   System* CurSystem,
                                   std::vector<std::shared_ptr<Job>> Unplanned,
                                   int CSearch = 1,
                                   int Iteration = 1);
    int Check(std::shared_ptr<Job> CurJob,
              std::shared_ptr<PC> PCForPlan,
              System* CurSystem,
              double NewPPoint);
    void UnPlan (std::vector<std::shared_ptr<Job>> Planned,
                 System * CurSystem,
                 std::vector<std::shared_ptr<Job>> Unplanned);
    double UpdatePPoint(System* CurSystem, std::vector<std::shared_ptr<Job>> Unplanned);
    void Plan (std::shared_ptr<Job> CurJob,
               std::shared_ptr<PC> CurPC,
               std::vector<std::shared_ptr<Job>> Planned, 
               System * CurSystem,
               std::vector<std::shared_ptr<Job>> Unplanned);
    void UpdateBList(System* CurSystem, std::shared_ptr<Job> CurJob);
    void UpdateFList(System* CurSystem, std::shared_ptr<Job> CurJob);
    void UpdateRList(std::shared_ptr<Job> CurJob);
    double LimitForPC; // порог для отсечения ядер при формировании итогового списка, как в основном алгоритме
    double CritLimit;
};