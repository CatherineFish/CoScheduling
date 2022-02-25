#pragma once 
#include <vector>
#include <memory>
#include <map>

#include "CoefClasses.h"

void Swap(int* a, int* b);
int PairGCD(int a, int b);
int PairLCM(int a, int b);
int LCM(std::vector<int> Periods);

class PC 
{
public:
    int ModNum;
    double PC_PPoint = 0;
public:
    PC(int Num): ModNum(Num) {}
    ~PC() = default;  
};

class Message;

class Task
{
public:
    double Period;
    double Time;
    double Left;
    double Right; 
public:
    int JobInit;
    double Slack;
    std::vector<int> OutMessage;
    std::vector<int> InMessage;
    std::vector<std::shared_ptr<Message>> MesOut;
    std::vector<std::shared_ptr<Message>> MesIn;
    Task (int Period_ = 0, 
          int Time_ = 0, 
          int Left_ = 0, 
          int Right_ = 0): Period(Period_), Time(Time_), Left(Left_), Right(Right_) {}
    ~Task() = default;    
};

class Job: public Task
{
public:
    int Num;
    double Start;
    std::shared_ptr<PC> JobPC; // сделать просто номером
    bool IsPlanned = false;
public:
    Job(int Num_): Num(Num_){}
    
    ~Job() = default;
    double Slack;
    std::vector<std::shared_ptr<Job>> JobTo;
    std::vector<std::shared_ptr<Job>> JobFrom;
    
    std::map<std::shared_ptr<PC>, double> ListBandwidth; // сделать просто номером 
    std::map<std::shared_ptr<PC>, double> ListFill; // сделать просто номером 
    std::map<std::shared_ptr<PC>, double> ListResult; // сделать просто номером 
};

class Message
{
public:
    std::shared_ptr<Task> Src; // просто номер
    std::shared_ptr<Task> Dest; // просто номер
    int SrcNum;
    int DestNum;
    double Size;
    double Bandwidth = 0.0;
    double Dur = 0.0;
public:
    Message(std::shared_ptr<Task> Src_, std::shared_ptr<Task> Dest_, double Size_): Size(Size_) 
    {
        Src = std::shared_ptr<Task>(Src_);
        Dest = std::shared_ptr<Task>(Dest_);
    }
    ~Message() = default;  
};

class ContextMessage: public Message
{
public:
    Job Src; // просто номер
    Job Dest; // просто номер
public:
    ContextMessage();
    ~ContextMessage() = default;  
};

class System
{
public:
    System(char * FileName);
    double PPoint = 0.0;
    double BTotal;
    double LCMPeriod;
    std::vector<std::shared_ptr<PC>> SystemPC;
    std::vector<std::shared_ptr<Task>> SystemTask;
    std::vector<std::shared_ptr<Message>> SystemMessage;
    void PrintSystem();
    ~System() = default;
    BLackCoef CurBLackCoef;
};