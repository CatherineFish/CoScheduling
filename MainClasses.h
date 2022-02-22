#pragma once 
#include <vector>

class PC 
{
public:
    int ModNum;
    double PC_PPoint = 0;
public:
    PC(int Num): ModNum(Num) {}
    ~PC() = default;  
};

class Task
{
public:
    double Period;
    double Time;
    double Left;
    double Right; 
public:
    std::vector<int> OutMessage;
    Task (int Period_, 
          int Time_, 
          int Left_, 
          int Right_): Period(Period_), Time(Time_), Left(Left_), Right(Right_) {}
    ~Task() = default;    
};

class Job: public Task
{
public:
    int Num;
    double Start;
    PC JobPC; // сделать просто номером
public:
    Job();
    ~Job() = default;
    double Slack;
    std::vector<std::pair<PC, double>> ListBandwidth; // сделать просто номером 
    std::vector<std::pair<PC, double>> ListFill; // сделать просто номером 
    std::vector<PC> ListResult; // сделать просто номером 
};

class Message
{
public:
    int Src; // просто номер
    int Dest; // просто номер
    double Size;
    double Bandwidth = 0.0;
    double Dur = 0.0;
public:
    Message(int Src_, 
            int Dest_, 
            double Size_): Src(Src_), Dest(Dest_), Size(Size_) {}
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
    std::vector<PC> SystemPC;
    std::vector<Task> SystemTask;
    std::vector<Message> SystemMessage;
    void PrintSystem();
    ~System() = default;
};