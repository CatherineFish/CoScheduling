
#include <cmath>
#include <vector>
#include <random>
#include <map>
#include <iostream>
#include <numeric>
#include <iterator>
#include <ctime>
#include <algorithm>

#include "InitialGenerator.h"
#include "../MainClasses.h"
#include "../TinyXml/tinyxml.h"
#include "../TinyXml/tinystr.h"


InitialGenerator :: InitialGenerator(char * Filename, int LCM_, int TaskNum_, int PCNum_, int ModNum_, int MesNum_, int MaxSize):
    TaskNum(TaskNum_), PCNum(PCNum_), ModNum(ModNum_), MesNum(MesNum_)
{
    std::srand(std::time(nullptr));
    
    LCM = int(std::pow(2, LCM_));
    
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::cout << "1" <<std::endl;
    std::normal_distribution<double> d{LCM_ / 2.0, LCM_ / 6.0};
    //TODO какая должна быть дисперсия?
    
    std::map<int, int> distribute{};
    for(int n = 0; n < TaskNum;) {
        int number = std::round(d(gen));
        if ((number >= 2) && (number <= LCM_)) {
            ++distribute[number];
            n++;
        }
    }
    std::cout << "2" <<std::endl;
    
    for (int i = 0; i < ModNum; i++)
    {
        ModVec.emplace_back(std::vector<int>{});
        ModVec[i].push_back(i);
    }
    std::cout << ModNum << std::endl;
    
    for(int n = ModNum; n < PCNum; n++) {
        ModVec[std::rand() % ModNum].push_back(n);
        
    }
    std::cout << "3" <<std::endl;
    
    std::cout << "ALL: " << std::endl;
    for(auto p : distribute) {
        std::cout << p.first << ' ' << std::string(p.second, '*') << '\n';
    }

    std::vector<std::map<int, int>> PCandTask;
    for (int i = 0; i < PCNum; i++)
    {
        PCandTask.push_back(std::map<int, int>{});
    }
    std::cout << PCNum << std::endl;
    int k = 0;
    // Это равномерное распределение задач по ядрам
        
    for(auto CurTask : distribute) {
        int ForAll = CurTask.second / PCNum;
        for (size_t i = 0; i < PCandTask.size(); i++)
        {
            PCandTask[i][CurTask.first] = ForAll;
        }

        for (int i = k; 
             i < std::min(k + CurTask.second - ForAll * PCandTask.size(), PCandTask.size()); 
             i++)
        {
            PCandTask[i][CurTask.first]++;
        
        }
        int Add = k + CurTask.second - ForAll * PCandTask.size() - PCandTask.size();
        for (int  i = 0; i < Add; i++)
        {
            PCandTask[i][CurTask.first]++;
        }
        k++;
        if (k >= PCandTask.size())
            k = 0;
    }

    std::cout << "CP: " << std::endl;
    int sum = 0;
    for (int i = 0; i < PCNum; i++)
    {
        sum = 0;
        for(auto p : PCandTask[i]) {
            std::cout << p.first << ' ' << std::string(p.second, '*') << '\n';
            sum += p.second;
        }
        if (sum == 0) {
            exit(0);
        }
        std::cout << std::endl;
    
    }

    for (size_t i = 0; i < PCandTask.size(); i++)
    {
        GeneratorForPC(PCandTask[i], LCM_);
    }
    std::cout << "MAx Lenght = " << MaxLenght << std::endl;
    for (size_t i = 0; i < PCandTask.size(); i++)
    {
        GeneratorForPC2(PCandTask[i], LCM_);
    }
    
    GeneratorForMessage(MaxSize);
    std::random_device rd2{};
    std::mt19937 gen2{rd2()}; 
    //TODO random вынести в класс
    std::normal_distribution<double> d2{MaxSize / 2.0, MaxSize / 6.0};
    
    /*int bb = 0;
    for (int i = 0; i < PtrAllPeriods.size(); i++)
    {
        bb = 0;
        for (int j = 0; j < PtrAllPeriods[i].size(); j++)
        {
            for (int k = 0; k < PtrAllPeriods[i][j].size(); k++)
            {
                if (PtrAllPeriods[i][j][k]->Period == 0) continue;
                AllJobs.emplace_back(std::make_shared<Job>(PtrAllPeriods[i][j][k]->Period * MaxLenght,
                                                           PtrAllPeriods[i][j][k]->Right - PtrAllPeriods[i][j][k]->Left,
                                                           int(bb / (PtrAllPeriods[i][j][k]->Period * MaxLenght)) * PtrAllPeriods[i][j][k]->Period * MaxLenght,
                                                           int(bb / (PtrAllPeriods[i][j][k]->Period * MaxLenght)) * PtrAllPeriods[i][j][k]->Period * MaxLenght + PtrAllPeriods[i][j][k]->Period * MaxLenght,
                                                           0, 0, 0));
                AllJobs[AllJobs.size() - 1]->Start = int(bb / (PtrAllPeriods[i][j][k]->Period * MaxLenght)) * PtrAllPeriods[i][j][k]->Period * MaxLenght  + PtrAllPeriods[i][j][k]->Left;
                AllJobs[AllJobs.size() - 1]->InitLeft = int(bb / (PtrAllPeriods[i][j][k]->Period * MaxLenght)) * PtrAllPeriods[i][j][k]->Period * MaxLenght;
                AllJobs[AllJobs.size() - 1]->InitRight = AllJobs[AllJobs.size() - 1]->InitLeft + PtrAllPeriods[i][j][k]->Period * MaxLenght;
                AllJobs[AllJobs.size() - 1]->JobInit = PtrAllPeriods[i][j][k]->JobInit;
                AllJobs[AllJobs.size() - 1]->NumOfTask = std::find(AllTasks.begin(), AllTasks.end(), PtrAllPeriods[i][j][k]) - AllTasks.begin();
                std::cout << "New Job " << AllJobs.size() - 1 << std::endl;
                std::cout << "bb " << bb << " Left " << PtrAllPeriods[i][j][k]->Left << std::endl;
                
                std::cout << "Start " << AllJobs[AllJobs.size() - 1]->Start << std::endl;
                
                std::cout << "Period " << AllJobs[AllJobs.size() - 1]->Period << std::endl;
                std::cout << "Time " << AllJobs[AllJobs.size() - 1]->Time << std::endl;
                std::cout << "Left " << AllJobs[AllJobs.size() - 1]->Left << std::endl;
                std::cout << "Right " << AllJobs[AllJobs.size() - 1]->Right << std::endl;
                std::cout << "InitLeft " << AllJobs[AllJobs.size() - 1]->InitLeft << std::endl;
                std::cout << "InitRight " << AllJobs[AllJobs.size() - 1]->InitRight << std::endl;
                std::cout << "NumOfTask " << AllJobs[AllJobs.size() - 1]->NumOfTask << std::endl;
                
                std::cout << std::endl;
                         
            }
            bb += 2 * MaxLenght;
            
        }
    }*/
    LCM *= MaxLenght;
    

    //std::cout << "----------------------------------" << std::endl;
    double MinDur = -1.0;
    for (size_t i = 0; i < AllTasks.size(); i++)
    {
        AllTasks[i]->Time = AllTasks[i]->Right - AllTasks[i]->Left;
        AllTasks[i]->Left = 0;
        AllTasks[i]->Period *= MaxLenght;
        
        AllTasks[i]->Right = AllTasks[i]->Period;
        int CMessageSize = int(std::round(d2(gen2)));
        while (CMessageSize < 0 || CMessageSize > MaxSize)
        {
            CMessageSize = int(std::round(d2(gen2)));
        }
        AllTasks[i]->CMessageSize = CMessageSize;
        MinDur = MinDur < 0.0 || MinDur < AllTasks[i]->Time ? AllTasks[i]->Time : MinDur; 
        /*std::cout << "Task " << i << " : " << std::endl;
        std::cout << "Left : " << AllTasks[i]->Left << std::endl;
        std::cout << "Right : " << AllTasks[i]->Right << std::endl;
        std::cout << "Time : " << AllTasks[i]->Time << std::endl;
        std::cout << "----------------------------------" << std::endl;*/
           
    }
    //std::cout << "================================" << std::endl;
    int ii, jj;
    for (const auto & CurMes: Mes)
    {
        //std::cout << "SRC " << CurMes->SrcNum << " Dest " << CurMes->DestNum << std::endl;
        for (int i = 0; i < int(LCM / AllTasks[CurMes->SrcNum]->Period); i++)
        {
            //std::cout << "i = " << i << std::endl;
            ii = -1;
            jj = -1;
            for (int k = 0; k < AllJobs.size(); k++)
            {
                //std::cout << "k = " << k << std::endl;
                if (ii != i && AllJobs[k]->NumOfTask == CurMes->SrcNum)
                {
                    //std::cout << "found src" << std::endl;
                    ii++;
                }
                if (jj != i && AllJobs[k]->NumOfTask == CurMes->DestNum)
                {
                    //std::cout << "found dst" << std::endl;
                    
                    jj++;
                }
                if (ii == i && jj == i)
                {
                    //std::cout << "found All" << std::endl;
                    
                    break;
                }
            }
            JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(AllJobs[ii], AllJobs[jj])] = std::shared_ptr(CurMes);
        }
    }

    for (size_t i = 0; i < Mes.size(); i++)
    {
        std::cout << "Message " << i << std::endl;
        std::cout << Mes[i]->SrcNum << " " << Mes[i]->DestNum << std::endl;
        std::cout << "================================" << std::endl;
    
    }
    int nn = 0;
    for (const auto & [Jobs, Mes] : JobMessage)
    {
        auto f = std::find(AllJobs.begin(), AllJobs.end(), Jobs.first) - AllJobs.begin();
        auto s = std::find(AllJobs.begin(), AllJobs.end(), Jobs.second) - AllJobs.begin();
        std::cout << f << " " << s << " " << nn << std::endl;
        nn++;
    }
    std::vector<int> Modules (ModNum);
    for (size_t i = 0; i < ModNum; i++)
    {
        Modules[i] = ModVec[i].size();
    }
    std::cout << "Modules" << std::endl;
    for (size_t i = 0; i < Modules.size(); i++)
    {
        std::cout << Modules[i] << std::endl;
    }
    
    std::cout << Band << std::endl;
    
    Band = Band == 0.0 ? MesNum * ((int(std::round(d2(gen2))) % MaxSize) / MinDur) : Band * 2;
    //TODO тут тоньше настройка
    std::cout << Band << std::endl;
    ResultCheck();
    GenerateInputFile (Filename, Modules);



}


void InitialGenerator :: GeneratorForMessage(int MaxSize)
{
    std::cout << "GEn Messages" << std::endl;
    std::srand(std::time(nullptr));
    std::random_device rd{};
    std::mt19937 gen{rd()}; 
    std::normal_distribution<double> d{MaxSize / 2.0, MaxSize / 6.0};
            //TODO какая должна быть дисперсия?
     
    int iter = 0;
    for (int n = 0; n < MesNum; )
    {   
        int PCFromIdx = std::rand() % PCNum;
        while (AllPeriods[PCFromIdx].size() == 0) {
            PCFromIdx = std::rand() % PCNum;
        }
        int PeriodFromIdx = std::rand() % AllPeriods[PCFromIdx].size();
        //Это номер, позже него ищем

        while (AllPeriods[PCFromIdx][PeriodFromIdx].size() == 0) {
            PeriodFromIdx = std::rand() % AllPeriods[PCFromIdx].size();
        }
        int NumInPeriod = std::rand() % AllPeriods[PCFromIdx][PeriodFromIdx].size();
        //Это период
        
        int CurTask = AllPeriods[PCFromIdx][PeriodFromIdx][NumInPeriod];
        if (CurTask == 0) continue;
        std::cout << "PC : " << PCFromIdx << " Period : " << PeriodFromIdx << " Num : " << NumInPeriod << " TaskPeriod : " << CurTask << std::endl;
        int PCToIdx = std::rand() % PCNum;
        //выбираем период
        
        int PeriodUntil = CurTask;
        PeriodUntil = std::min(PeriodUntil, int(AllPeriods[PCToIdx].size()));
        std::cout << "PC TO : " << PCToIdx << " PeriodUntil : " << PeriodUntil << std::endl;
        bool found = false;
        int FromPC, FromPeriod, FromNum, ToPC, ToPeriod, ToNum;
        for (int i = PeriodFromIdx > CurTask ? 0 : PeriodFromIdx; i < PeriodUntil; i++)
        {
            for (size_t j = 0; j < AllPeriods[PCToIdx][i].size(); j++)
            {
                //TODO задавать и внкутри модульные сообщения
                std::cout << i << " " << j << "  " << AllPeriods[PCToIdx][i][j] << std::endl;
                if (AllPeriods[PCToIdx][i][j] == CurTask)
                {
                    if (PtrAllPeriods[PCToIdx][i][j]->Left > PtrAllPeriods[PCFromIdx][PeriodFromIdx][NumInPeriod]->Right)
                    {
                        FromPC = PCFromIdx;
                        FromPeriod = PeriodFromIdx;
                        FromNum = NumInPeriod;
                        ToPC = PCToIdx;
                        ToPeriod = i;
                        ToNum = j; 
                        found = true;  
                        break;
                    } else if (PtrAllPeriods[PCToIdx][i][j]->Right < PtrAllPeriods[PCFromIdx][PeriodFromIdx][NumInPeriod]->Left)
                    {
                        ToPC = PCFromIdx;
                        ToPeriod = PeriodFromIdx;
                        ToNum = NumInPeriod;
                        FromPC = PCToIdx;
                        FromPeriod = i;
                        FromNum = j; 
                        found = true;  
                        break;
                    }
                    
                }
            }
            /*
            for (size_t j = (i == PeriodFromIdx || i == 0) ? NumInPeriod + 1 : 0; 
                 j < AllPeriods[PCToIdx][i].size(); j++)
            {
                std::cout << i << " " << j << "  " << AllPeriods[PCToIdx][i][j] << std::endl;
                if (AllPeriods[PCToIdx][i][j] == CurTask)
                {
                    found = true;  
                    Idx = i;
                    Jdx = j;  
                    break;
                }
            }
            */
        }
    
        if (found) {
            
            int MessageSize = int(std::round(d(gen)));
            while (MessageSize < 0 || MessageSize > MaxSize) {
                MessageSize = int(std::round(d(gen)));
            }
            MessageSize %= MaxSize;
            auto CurMes = std::make_shared<Message>(std::shared_ptr<Task>(PtrAllPeriods[FromPC][FromPeriod][FromNum]),
                                                    std::shared_ptr<Task>(PtrAllPeriods[ToPC][ToPeriod][ToNum]),
                                                    MessageSize);
            CurMes->SrcNum = std::find(AllTasks.begin(), AllTasks.end(), CurMes->Src) - AllTasks.begin();
            CurMes->DestNum = std::find(AllTasks.begin(), AllTasks.end(), CurMes->Dest) - AllTasks.begin();
            
            Mes.push_back(std::shared_ptr<Message>(CurMes));
            
            n++;
            iter = 0;
            bool f1 = false, f2 = false;
            int m1, m2;
            for (int jj = 0; jj < ModVec.size() && !f1 && !f2; jj++)
            {
                if (!f1)
                {
                    if (std::find(ModVec[jj].begin(), ModVec[jj].end(), FromPC) != ModVec[jj].end())
                        m1 = jj;
                }
                if (!f2)
                {
                    if (std::find(ModVec[jj].begin(), ModVec[jj].end(), ToPC) != ModVec[jj].end())
                        m2 = jj;
                }
            }
            PtrAllPeriods[FromPC][FromPeriod][FromNum]->JobInit = m1;
            PtrAllPeriods[ToPC][ToPeriod][ToNum]->JobInit = m2;
            std::cout << CurMes->Size << " " <<  PtrAllPeriods[ToPC][ToPeriod][ToNum]->Left - PtrAllPeriods[FromPC][FromPeriod][FromNum]->Right << " " <<
                         CurMes->Size / (PtrAllPeriods[ToPC][ToPeriod][ToNum]->Left - PtrAllPeriods[FromPC][FromPeriod][FromNum]->Right) << " " <<
                         Band << std::endl;   
            Band += CurMes->Size / (PtrAllPeriods[ToPC][ToPeriod][ToNum]->Left - PtrAllPeriods[FromPC][FromPeriod][FromNum]->Right);
            std::cout << Band << std::endl;
            CurMes->Bandwidth = CurMes->Size / (PtrAllPeriods[ToPC][ToPeriod][ToNum]->Left - PtrAllPeriods[FromPC][FromPeriod][FromNum]->Right);
            //if (m1 != m2) {
            // три верхних строки раньше были тут   
            //}
        } else {
            iter++;
            if (iter > 100)
            {
                std::cout << "Problems. Messages have " << n << std::endl;
                exit(1);
                break;
            }
        }

        
    }

}


void InitialGenerator :: GeneratorForPC(std::map<int, int> TaskMap, int LCM_)
{

    static int PC = 0;
    static int Bias = 0;
    std::cout << std::endl << "PC NUM : " << PC << std::endl;
    int NumOfPeriod = LCM / std::pow(2, (*TaskMap.begin()).first);
    std::vector<int> k (LCM_);
    std::iota (std::begin(k), std::end(k), 0);
    AllPeriods.push_back(std::vector<std::vector<int>> {});
    PtrAllPeriods.push_back(std::vector<std::vector<std::shared_ptr<Task>>>{});
    
    for (int i = 0; i < NumOfPeriod; i++)
    {
        AllPeriods[PC].push_back(std::vector<int>{});
        PtrAllPeriods[PC].push_back(std::vector<std::shared_ptr<Task>>{});
    }
    std::vector <std::pair<int, int>> TasksForPlan;
    for (auto CurTask : TaskMap)
    {
        TasksForPlan.emplace_back(std::pair<int, int>(CurTask.first, CurTask.second));
    }
    if (Bias >= TasksForPlan.size())
    {
        Bias = 0;
    }    

    //Вариант со смещением  
    for (int ii = Bias; ii < TasksForPlan.size(); ii++)
    {
        auto CurTask = TasksForPlan[ii];
        for (int j = 0; j < CurTask.second; j++)
        {
            AllTasks.push_back(std::make_shared<Task>(std::pow(2, CurTask.first)));
        
            int add = k[CurTask.first / (*TaskMap.begin()).first - 1];
            std::cout << "cur period = " << CurTask.first << " add = " << add << std::endl;
            for (int i = 0; i < NumOfPeriod; i++)
            {
                if (i == add)
                {
                    AllPeriods[PC][i].push_back(CurTask.first);
                    PtrAllPeriods[PC][i].push_back(std::shared_ptr<Task>(AllTasks[AllTasks.size() - 1]));    
                    add += CurTask.first / (*TaskMap.begin()).first;
                } else {
                    AllPeriods[PC][i].push_back(0);
                    PtrAllPeriods[PC][i].push_back(std::make_shared<Task>(0));  

                }
            }
            k[CurTask.first / (*TaskMap.begin()).first - 1]++;
            if (k[CurTask.first / (*TaskMap.begin()).first - 1] > (CurTask.first / (*TaskMap.begin()).first) - 1)
                k[CurTask.first / (*TaskMap.begin()).first - 1] = 0;
        }
    }
    for (int ii = 0; ii < Bias; ii++)
    {
        auto CurTask = TasksForPlan[ii];
        for (int j = 0; j < CurTask.second; j++)
        {
            AllTasks.push_back(std::make_shared<Task>(std::pow(2, CurTask.first)));
        
            int add = k[CurTask.first / (*TaskMap.begin()).first - 1];
            for (int i = 0; i < NumOfPeriod; i++)
            {
                if (i == add)
                {
                    AllPeriods[PC][i].push_back(CurTask.first);
                    PtrAllPeriods[PC][i].push_back(std::shared_ptr<Task>(AllTasks[AllTasks.size() - 1]));    
                    add += CurTask.first / (*TaskMap.begin()).first;
                } else {
                    AllPeriods[PC][i].push_back(0);
                    PtrAllPeriods[PC][i].push_back(std::make_shared<Task>(0));  

                }
            }
            k[CurTask.first / (*TaskMap.begin()).first - 1]++;
            if (k[CurTask.first / (*TaskMap.begin()).first - 1] > (CurTask.first / (*TaskMap.begin()).first) - 1)
                k[CurTask.first / (*TaskMap.begin()).first - 1] = 0;
        }
    }
    Bias++;

    
    /*
    //Вариант без смещения
    for (auto CurTask : TaskMap)
    {
        for (int j = 0; j < CurTask.second; j++)
        {
            AllTasks.push_back(std::make_shared<Task>(std::pow(2, CurTask.first)));
        
            for (int i = k[CurTask.first - 1]; i < NumOfPeriod; i = i + CurTask.first)
            {

                AllPeriods[PC][i].push_back(CurTask.first);
                PtrAllPeriods[PC][i].push_back(std::shared_ptr<Task>(AllTasks[AllTasks.size() - 1]));    
            }
            k[CurTask.first - 1]++;
            if (k[CurTask.first - 1] > CurTask.first - 1)
                k[CurTask.first - 1] = 0;
        }
    }
    */

    //Просто печать
    for (size_t i = 0; i < AllPeriods[PC].size(); i++)
    {
        std::cout << "Period : " << i  << std::endl;
        for (size_t j = 0; j < AllPeriods[PC][i].size(); j++)
        {
            std::cout << AllPeriods[PC][i][j] << " ";
        }
        std::cout << std::endl;
    }

    
    for (size_t i = 0; i < PtrAllPeriods[PC].size(); i++)
    {
        if (MaxLenght < PtrAllPeriods[PC][i].size())
        {
            MaxLenght = PtrAllPeriods[PC][i].size();
        }
    }
    PC++;
    
}

void InitialGenerator :: GeneratorForPC2(std::map<int, int> TaskMap, int LCM_)
{
    static int PC = 0;
    std::cout << std::endl << "PC NUM : " << PC << std::endl;
    
    int MaxLenght2 = 0;
    
    for (size_t i = 0; i < PtrAllPeriods[PC].size(); i++)
    {
        if (MaxLenght2 < PtrAllPeriods[PC][i].size())
        {
            MaxLenght2 = PtrAllPeriods[PC][i].size();
        }
    }
    int Dur = std::floor((std::pow(2, (*TaskMap.begin()).first) * MaxLenght) / double(MaxLenght2)) ;
    std::cout << "Dur in this period = " << Dur << " " << (*TaskMap.begin()).first << " " << MaxLenght2 << std::endl;
    for (size_t i = 0; i < PtrAllPeriods[PC].size(); i++)
    {
        for (size_t j = 0; j < PtrAllPeriods[PC][i].size(); j++)
        {
            if (PtrAllPeriods[PC][i][j]->Right != 0 || PtrAllPeriods[PC][i][j]->Period == 0) continue;
            PtrAllPeriods[PC][i][j]->Left = j * Dur + i * std::pow(2, (*TaskMap.begin()).first) * MaxLenght;
            // - 
            //                         int(i * std::pow(2, (*TaskMap.begin()).first) / PtrAllPeriods[PC][i][j]->Period) * PtrAllPeriods[PC][i][j]->Period; 
            PtrAllPeriods[PC][i][j]->Right = PtrAllPeriods[PC][i][j]->Left + Dur;   
               
        }
        
    }

    
    
    //TODO тут сложнее надо генерить
    // сейчас все задачи одной длительности
    /*for (int i = 0; i < MaxLenght; i++)
    {
        PtrAllPeriods[PC][Idx][i]->Left = i * Dur + Idx * std::pow(2, (*TaskMap.begin()).first) - 
                                     int(Idx * std::pow(2, (*TaskMap.begin()).first) / PtrAllPeriods[PC][Idx][i]->Period) * PtrAllPeriods[PC][Idx][i]->Period; 
        PtrAllPeriods[PC][Idx][i]->Right = PtrAllPeriods[PC][Idx][i]->Left + Dur;   
        for (size_t j = 0; j < AllTasks.size(); j++)
        {
            std::cout << "(" << AllTasks[j]->Left << " " << AllTasks[j]->Right << " per : " << AllTasks[j]->Period << ") - " << j << "    ";
        }
        std::cout << std::endl;
        
    }*/

    

    for (size_t i = 0; i < PtrAllPeriods[PC].size(); i++)
    {
        std::cout << "Period : " << i + 1 << std::endl;
        for (size_t j = 0; j < PtrAllPeriods[PC][i].size(); j++)
        {
            std::cout << "(" << PtrAllPeriods[PC][i][j]->Left << ", " << PtrAllPeriods[PC][i][j]->Right << ")  ";
        }
        std::cout << std::endl;
    }
    
    int bb = 0;
    for (int j = 0; j < PtrAllPeriods[PC].size(); j++)
    {
        for (int k = 0; k < PtrAllPeriods[PC][j].size(); k++)
        {
            if (PtrAllPeriods[PC][j][k]->Period == 0) continue;
            AllJobs.emplace_back(std::make_shared<Job>(PtrAllPeriods[PC][j][k]->Period * MaxLenght,
                                                       PtrAllPeriods[PC][j][k]->Right - PtrAllPeriods[PC][j][k]->Left,
                                                       int(bb / (PtrAllPeriods[PC][j][k]->Period * MaxLenght)) * PtrAllPeriods[PC][j][k]->Period * MaxLenght,
                                                       int(bb / (PtrAllPeriods[PC][j][k]->Period * MaxLenght)) * PtrAllPeriods[PC][j][k]->Period * MaxLenght + PtrAllPeriods[PC][j][k]->Period * MaxLenght,
                                                       0, 0, 0));
            AllJobs[AllJobs.size() - 1]->Start = int(bb / (PtrAllPeriods[PC][j][k]->Period * MaxLenght)) * PtrAllPeriods[PC][j][k]->Period * MaxLenght  + PtrAllPeriods[PC][j][k]->Left;
            AllJobs[AllJobs.size() - 1]->InitLeft = int(bb / (PtrAllPeriods[PC][j][k]->Period * MaxLenght)) * PtrAllPeriods[PC][j][k]->Period * MaxLenght;
            AllJobs[AllJobs.size() - 1]->InitRight = AllJobs[AllJobs.size() - 1]->InitLeft + PtrAllPeriods[PC][j][k]->Period * MaxLenght;
            AllJobs[AllJobs.size() - 1]->JobInit = PtrAllPeriods[PC][j][k]->JobInit;
            AllJobs[AllJobs.size() - 1]->NumOfTask = std::find(AllTasks.begin(), AllTasks.end(), PtrAllPeriods[PC][j][k]) - AllTasks.begin();
            std::cout << "New Job " << AllJobs.size() - 1 << std::endl;
            std::cout << "bb " << bb << " Left " << PtrAllPeriods[PC][j][k]->Left << std::endl;
            
            std::cout << "Start " << AllJobs[AllJobs.size() - 1]->Start << std::endl;
            
            std::cout << "Period " << AllJobs[AllJobs.size() - 1]->Period << std::endl;
            std::cout << "Time " << AllJobs[AllJobs.size() - 1]->Time << std::endl;
            std::cout << "Left " << AllJobs[AllJobs.size() - 1]->Left << std::endl;
            std::cout << "Right " << AllJobs[AllJobs.size() - 1]->Right << std::endl;
            std::cout << "InitLeft " << AllJobs[AllJobs.size() - 1]->InitLeft << std::endl;
            std::cout << "InitRight " << AllJobs[AllJobs.size() - 1]->InitRight << std::endl;
            std::cout << "NumOfTask " << AllJobs[AllJobs.size() - 1]->NumOfTask << std::endl;
            
            std::cout << std::endl;
                     
        }
        bb += std::pow(2, (*TaskMap.begin()).first) * MaxLenght;
        
    }
    


    PC++;

    return;
}

void InitialGenerator :: GenerateInputFile (char *filename, 
                        std::vector<int> ModNum)
{

    
    TiXmlDocument doc;  
    TiXmlElement* msg;
    TiXmlElement * XMLSystem = new TiXmlElement("system");  
    doc.LinkEndChild(XMLSystem);  
    XMLSystem->SetDoubleAttribute("modnum", ModNum.size());
    XMLSystem->SetDoubleAttribute("tasknum", AllTasks.size());
    XMLSystem->SetDoubleAttribute("bandwidth", Band);
    XMLSystem->SetDoubleAttribute("mesnum", Mes.size());
    
    TiXmlElement * modules = new TiXmlElement("modules");
    XMLSystem->LinkEndChild(modules);
    TiXmlElement * pc = new TiXmlElement("pc"); 
    for (size_t i = 0; i < ModNum.size(); i++)
    {
        std::string s = std::to_string(ModNum[i]);
        pc->LinkEndChild(new TiXmlText(s.c_str()));
        modules->LinkEndChild(pc);
        pc = new TiXmlElement("pc");
    }

    TiXmlElement * tasks = new TiXmlElement("tasks");
    XMLSystem->LinkEndChild(tasks);
    TiXmlElement * task = new TiXmlElement("task"); 
    for (size_t i = 0; i < AllTasks.size(); i++)
    {
        task->SetDoubleAttribute("period", AllTasks[i]->Period);
        
        task->SetDoubleAttribute("time", AllTasks[i]->Time);
        task->SetDoubleAttribute("left", AllTasks[i]->Left);
        task->SetDoubleAttribute("right", AllTasks[i]->Right);
        task->SetDoubleAttribute("messize", AllTasks[i]->CMessageSize);
        tasks->LinkEndChild(task);
        task = new TiXmlElement("task");
    }

    TiXmlElement * messages = new TiXmlElement("messages");
    XMLSystem->LinkEndChild(messages);
    TiXmlElement * mes = new TiXmlElement("mes");   
    for (size_t i = 0; i < Mes.size(); i++)
    {
        mes->SetDoubleAttribute("src", Mes[i]->SrcNum);
        mes->SetDoubleAttribute("dest", Mes[i]->DestNum);
        mes->SetDoubleAttribute("size", Mes[i]->Size);
        messages->LinkEndChild(mes);
        mes = new TiXmlElement("mes");
    }
    doc.SaveFile(filename);
    return;  
} 

void InitialGenerator:: ResultCheck()
{
    

    double SumBand = 0.0;
    for (const auto & [Jobs, Mes] : JobMessage) {
        if (Mes->ResultPlanned && (Jobs.first->JobInit == Jobs.second->JobInit)) 
        {
            std::cout << "Problem: message for jobs on one module" << std::endl;
            std::cout << "Mes: " << Mes->Size << std::endl;
            
            //return;
        } else {
            std::cout << "OK Mes: " << Mes->Size << std::endl;    
        }
        if (!Mes->ResultPlanned && (Jobs.first->JobInit != Jobs.second->JobInit)) 
        {
            std::cout << "Problem: no message for jobs on different module" << std::endl;
            std::cout << "Job SRC: " << Jobs.first->Start << " Time : " << Jobs.first->Time <<
                         " Job DEST: " << Jobs.second->Time << " Time : " << Jobs.second->Start << std::endl;
            exit(1);;
        }
            
        if (Mes->ResultPlanned)
        {
            SumBand += Mes->Bandwidth;
        }
        if (Mes->ResultPlanned && Jobs.first->JobInit != Jobs.second->JobInit && Jobs.first->Start + Jobs.first->Time + Mes->Dur > Jobs.second->Start)
        {
            std::cout << "Problem: message arrives too late" << std::endl;
            std::cout << "Job SRC: " << Jobs.first->Time << " Time : " << Jobs.first->Start <<
                         " Job DEST: " << Jobs.second->Time << " Time : " << Jobs.second->Start << std::endl;
            exit(1);
        }
    }
    
    /*for (const auto & Mes : CurSystem->SystemCMessage) {
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
    }*/
    std::cout << "REs Band " << SumBand << std::endl; 
    
    if (SumBand > Band) 
    {
        std::cout << "Problem: messages have too much bandwidth" << std::endl;
        //exit(1);
    }
    for (const auto & Job : AllJobs)
    {
        if (Job->Start + Job->Time > Job->InitRight) 
        {
            std::cout << "Problem: job ends too late" << std::endl;
            std::cout << "Job: " << Job->Start << " Time: " << Job->Time << " Right : " << Job->InitRight << std::endl;
            exit(1);
        }
        if (Job->Start < Job->InitLeft) 
        {
            std::cout << "Problem: job starts too early" << std::endl;
            exit(1);
        }

    }
    std::cout << "CHECK RESULT : OK" << std::endl;
            
    return;

}
