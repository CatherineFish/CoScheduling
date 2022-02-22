#include <fstream>
#include <iostream>
#include "MainClasses.h"


System::System (char * FileName)
{
    std::ifstream InputFile(FileName);
    int ModNum, PCNum;
    int TaskNum, Period, Time, Left, Right, NMessage, Num;
    int MessageNum, Src, Dest, Size;
    InputFile >> ModNum;
    for (int i = 0; i < ModNum; i++)
    {
        InputFile >> PCNum;
        for (int j=0; j< PCNum; j++)
        {
            SystemPC.emplace_back(i);
        }        
    }
    InputFile >> TaskNum;
    for (int i = 0; i < TaskNum; i++)
    {
        InputFile >> Period >> Time >> Left >> Right >> NMessage;
        SystemTask.emplace_back(Period, Time, Left, Right);
        for (int j = 0; j < NMessage; j++)
        {
            InputFile >> Num; 
            SystemTask[i].OutMessage.emplace_back(Num);
        }
    }
    InputFile >> MessageNum;
    for (int i = 0; i < MessageNum; i++)
    {
        InputFile >> Src >> Dest >> Size;
        SystemMessage.emplace_back(Src, Dest, Size);
    }
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
        std::cout << "Module: " << SystemPC[i].ModNum << std::endl;
        std::cout << "PPoint: " << SystemPC[i].PC_PPoint << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "--------Task--------:" << std::endl;
    for (size_t i = 0; i < SystemTask.size(); i++)
    {
        std::cout << std::endl;
        std::cout << "     Task " << i << std::endl;
        std::cout << "Period: " << SystemTask[i].Period << std::endl;
        std::cout << "Time: " << SystemTask[i].Time << std::endl;
        std::cout << "Left: " << SystemTask[i].Left << std::endl;
        std::cout << "Right: " << SystemTask[i].Right << std::endl;
    }
    std::cout << std::endl;

    std::cout << "--------Message--------:" << std::endl;
    for (size_t i = 0; i < SystemMessage.size(); i++)
    {
        std::cout << std::endl;
        std::cout << "     Message " << i  << std::endl;
        std::cout << "Src: " << SystemMessage[i].Src << std::endl;
        std::cout << "Dest: " << SystemMessage[i].Dest << std::endl;
        std::cout << "Size: " << SystemMessage[i].Size << std::endl;
        std::cout << "Bandwidth: " << SystemMessage[i].Bandwidth << std::endl;
        std::cout << "Dur: " << SystemMessage[i].Dur << std::endl;
    }
    std::cout << std::endl;
    std::cout << "======================" << std::endl; 
    return;
}