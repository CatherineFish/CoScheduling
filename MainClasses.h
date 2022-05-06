#pragma once 
#include <vector>
#include <memory>
#include <map>

#include "CoefClasses.h"

void Swap(int* a, int* b); // вспомогательная функция для функции вычисление цикла планирования
int PairGCD(int a, int b); // вспомогательная функция для функции вычисление цикла планирования
int PairLCM(int a, int b); // вспомогательная функция для функции вычисление цикла планирования
int LCM(std::vector<int> Periods); // функция вычисление цикла планирования


class PC 
{
public:
    int Num; // сквозной номер ядра
    int ModNum; // номер модуля, в котором находится ядро
    double PC_PPoint = 0; // точка планирования на ядре
    std::vector<int> PlannedOnPC;
    PC(int Num): ModNum(Num) {}
    ~PC() = default;  
};

class Message;

class Task
{
public:
    double InitLeft = 0.0;
    double InitRight = 0.0;
    
    double Period;
    double Time;
    double Left;
    double Right; 
    double CMessageSize;
    double MinLeft;
    double MaxLeft;
    bool IsInit = false;
    int JobInit;
    std::vector<int> OutMessage;
    std::vector<int> InMessage;
    std::vector<std::weak_ptr<Message>> MesOut;
    std::vector<std::weak_ptr<Message>> MesIn;
    Task (int Period_ = 0, 
          double Time_ = 0, 
          double Left_ = 0, 
          double Right_ = 0,
          double CMessageSize_ = 0.0): Period(Period_), 
                                        Time(Time_), 
                                        Left(Left_), 
                                        Right(Right_),
                                        CMessageSize(CMessageSize_) {}
    ~Task() = default;    
};

class Job: public Task
{
public:
    int Num;
    int NumOfTask;
    double Start;
    std::shared_ptr<PC> JobPC;
    bool IsPlanned = false;
    std::shared_ptr<Job> PreviousJob;
    Job(int Period_ = 0, 
        double Time_ = 0, 
        double Left_ = 0, 
        double Right_ = 0,
        double CMessageSize_ = 0.0,
        int Num_ = 0,
        int NumOfTask_ = 0): Task(Period_, Time_, Left_, Right_, CMessageSize_), 
                             Num(Num_),
                             NumOfTask(NumOfTask_){}
    ~Job() = default;
    double Slack;
    std::map<std::shared_ptr<PC>, double> ListBandwidth;
    std::map<std::shared_ptr<PC>, double> ListFill; 
    std::multimap<double, std::shared_ptr<PC>> ListResult; 
    std::vector<double> NewLimitForPlan; 
    std::vector<bool> isUpdated;
};

class Message
{
public:
    std::shared_ptr<Task> Src;
    std::shared_ptr<Task> Dest;
    int SrcNum;
    int DestNum;
    double Size;
    double Bandwidth = 0.0;
    double Dur = 0.0;
    StableCoef StabilityCoef;
    Message(std::shared_ptr<Task> Src_, std::shared_ptr<Task> Dest_, double Size_): Size(Size_) 
    {
        Src = std::shared_ptr<Task>(Src_);
        Dest = std::shared_ptr<Task>(Dest_);
    }
    Message(){}
    ~Message() = default;
    bool ResultPlanned = true;  
};

class ContextMessage: public Message
{
public:
    std::shared_ptr<Job> Src;
    std::shared_ptr<Job> Dest;
    ContextMessage(std::shared_ptr<Job> Src_, std::shared_ptr<Job> Dest_) 
    {
        Src = std::shared_ptr<Job>(Src_);
        Dest = std::shared_ptr<Job>(Dest_);
    }
    ~ContextMessage() = default;  
};

class System
{
public:
    System(char * FileName);
    double PPoint = 0.0;
    double BTotal;
    double CurBand = 0.0;
    double LCMPeriod;
    std::vector<std::shared_ptr<PC>> SystemPC;
    std::vector<std::shared_ptr<Task>> SystemTask;
    std::vector<std::shared_ptr<Message>> SystemMessage;
    std::vector<std::shared_ptr<ContextMessage>> SystemCMessage;
    std::vector<std::shared_ptr<Job>> SystemJob;
    std::map<std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>, std::shared_ptr<Message>> JobMessage;
    void PrintMessages();
    void PrintPC();
    void PrintSystem();
    ~System() = default;
    void ExportToDotFile(char * filename);
    BLackCoef CurBLackCoef;
};