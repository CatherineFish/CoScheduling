#include <fstream>
#include <iostream>
#include "MainClasses.h"

void Swap(int* a, int* b) 
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
    return;
}

int PairGCD(int a, int b) 
{
    if (a < b) {
        Swap(&a, &b);
    }
    while (a % b != 0) {
        a = a % b;
        Swap(&a, &b);
    }
    return b;
}

int PairLCM(int a, int b) 
{
    return (a * b) / PairGCD(a, b);
}

int LCM(std::vector<int> Periods) 
{
    if (Periods.size() == 0)
    {
        return -1;
    }
    if (Periods.size() == 1)
    {
        return Periods[0];
    }
    int Result = PairLCM(Periods[0], Periods[1]);
    for (size_t i = 2; i < Periods.size(); i++) 
    {
        Result = PairLCM(Result, Periods[i]);
    }
    return Result;
}


System::System (char * FileName)
{
    std::ifstream InputFile(FileName);
    int ModNum, PCNum, k = 0;
    int TaskNum, Period, Time, Left, Right, NMessage, Num;
    double CMessageSize;
    int MessageNum, Src, Dest, Size;
    std::vector<int> PeriosVector;
    InputFile >> ModNum;
    for (int i = 0; i < ModNum; i++)
    {
        InputFile >> PCNum;
        for (int j=0; j< PCNum; j++)
        {
            SystemPC.push_back(std::make_shared<PC>(i));
            SystemPC[k]->Num = k;
            k++;
        }        
    }
    InputFile >> TaskNum;
    for (int i = 0; i < TaskNum; i++)
    {
        InputFile >> Period >> Time >> Left >> Right >>CMessageSize;
        PeriosVector.emplace_back(Period);
        SystemTask.push_back(std::make_shared<Task>(
                             Period, Time, Left, Right, CMessageSize));
    }
    InputFile >> BTotal;    
    InputFile >> MessageNum;
    
    for (int i = 0; i < MessageNum; i++)
    {
        InputFile >> Src >> Dest >> Size;
        SystemMessage.push_back(std::make_shared<Message>(
                                SystemTask[Src], SystemTask[Dest], Size));
        SystemMessage[i]->IsPlanned = true;
        SystemTask[Src]->OutMessage.emplace_back(Dest);
        SystemTask[Src]->MesOut.push_back(std::weak_ptr<Message>(SystemMessage[i]));
        SystemTask[Dest]->InMessage.emplace_back(Src);
        //SystemTask[Dest]->MesIn.emplace_back(std::shared_ptr<Message>(SystemMessage[i]));
        SystemMessage[i]->DestNum = Dest;
        SystemMessage[i]->SrcNum = Src;
    }
    LCMPeriod = LCM(PeriosVector);
    
    
    return;
}


void System:: PrintSystem()
{
    std::cout << "========SYSTEM========" << std::endl; 
    std::cout << "--------PC--------:" << std::endl;
    for (size_t i = 0; i < SystemPC.size(); i++)
    {
        std::cout << std::endl;
        std::cout << "       PC " << i << std::endl;
        std::cout << "Module: " << SystemPC[i]->ModNum << std::endl;
        std::cout << "PPoint: " << SystemPC[i]->PC_PPoint << std::endl;
        std::cout << "Num: " << SystemPC[i]->Num << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "--------Task--------:" << std::endl;
    std::cout << "LCM = " << LCMPeriod << std::endl;
    for (size_t i = 0; i < SystemTask.size(); i++)
    {
        std::cout << std::endl;
        std::cout << "     Task " << i << std::endl;
        std::cout << "Period: " << SystemTask[i]->Period << std::endl;
        std::cout << "Time: " << SystemTask[i]->Time << std::endl;
        std::cout << "Left: " << SystemTask[i]->Left << std::endl;
        std::cout << "Right: " << SystemTask[i]->Right << std::endl;
        std::cout << "Input Mes Count: " << SystemTask[i]->InMessage.size() << std::endl;
        std::cout << "Output Mes Count: " << SystemTask[i]->OutMessage.size() << std::endl;
        std::cout << "Output Mes: " << std::endl;
        for (size_t j = 0; j < SystemTask[i]->MesOut.size(); j++)
        {
            std::shared_ptr<Message> p = SystemTask[i]->MesOut[j].lock();    
            if (p) 
            {
                std::cout << p->Size << " ";
            } else 
            {
                std::cout << "PROBLEMS" << std::endl;
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "--------Message--------:" << std::endl;
    for (size_t i = 0; i < SystemMessage.size(); i++)
    {
        std::cout << std::endl;
        std::cout << "     Message " << i  << std::endl;
        std::cout << "Src: " << SystemMessage[i]->Src->Time << std::endl;
        std::cout << "Dest: " << SystemMessage[i]->Dest->Time << std::endl;
        std::cout << "Size: " << SystemMessage[i]->Size << std::endl;
        std::cout << "Bandwidth: " << SystemMessage[i]->Bandwidth << std::endl;
        std::cout << "Dur: " << SystemMessage[i]->Dur << std::endl;
    }
    std::cout << std::endl;
    std::cout << "======================" << std::endl; 
    return;
}