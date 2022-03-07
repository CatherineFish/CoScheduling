#pragma once 
#include <vector>
#include <memory>
#include <map>

#include "CoefClasses.h"

void Swap(int* a, int* b);
int PairGCD(int a, int b);
int PairLCM(int a, int b);
int LCM(std::vector<int> Periods);


class Job; 

class PC 
{
public:
    int Num;
    int ModNum;
    double PC_PPoint = 0;
    std::vector<int> PlannedOnPC;
public:
    PC(int Num): ModNum(Num) {}
    ~PC() = default;  
};

class Message;

class Task
{
public:
    double InitLeft = 0.0;
    
    double Period;
    double Time;
    double Left;
    double Right; 
    double CMessageSize;
public:
    int JobInit;
    double Slack;
    std::vector<int> OutMessage;
    std::vector<int> InMessage;
    std::vector<std::weak_ptr<Message>> MesOut;
    std::vector<std::weak_ptr<Message>> MesIn;
    Task (int Period_ = 0, 
          int Time_ = 0, 
          int Left_ = 0, 
          int Right_ = 0,
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
    double BandForCMes;
    std::shared_ptr<PC> JobPC; // сделать просто номером
    bool IsPlanned = false;
    double IsCorrected = 0.0;
    std::shared_ptr<Job> PreviousJob;
public:
    Job(int Num_): Num(Num_){}
    
    ~Job() = default;
    double Slack;
    //std::vector<std::shared_ptr<Job>> JobTo;
    //std::vector<std::shared_ptr<Job>> JobFrom;
    
    std::map<std::shared_ptr<PC>, double> ListBandwidth; // сделать просто номером 
    std::map<std::shared_ptr<PC>, double> ListFill; // сделать просто номером 
    std::multimap<double, std::shared_ptr<PC>> ListResult; // сделать просто номером 
};

class Message
{
public:
    std::shared_ptr<Task> Src; // просто номер
    std::shared_ptr<Task> Dest; // просто номер
    int SrcNum;
    bool NotModify = false;
    int DestNum;
    double Size;
    double Bandwidth = 0.0;
    double Dur = 0.0;
    double TmpDur = 1.0;
    bool IsPlanned = false;
    StableCoef StabilityCoef;
public:
    Message(std::shared_ptr<Task> Src_, std::shared_ptr<Task> Dest_, double Size_): Size(Size_) 
    {
        Src = std::shared_ptr<Task>(Src_);
        Dest = std::shared_ptr<Task>(Dest_);
    }
    Message(){}
    ~Message() = default;  
};

class ContextMessage: public Message
{
public:
    std::shared_ptr<Job> Src; // просто номер
    std::shared_ptr<Job> Dest; // просто номер
    
public:
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
    BLackCoef CurBLackCoef;
};