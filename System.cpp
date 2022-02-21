#include <fstream>
#include "MainClasses.h"

class System
{
public:
    SystemTask(char * FileName);
    std::vector<PC> SystemPC;
    std::vector<Task> SystemTask;
    std::vector<Message> SystemTask;
    ~SystemTask() {};
};

System:: System (char * FileName)
{
    std::ifstream InputFile(FileName);
    int ModNum, PCNum;
    int TaskNum, Period, Time, Left, Right, NMessage;
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
        SystemTask.emplace_back(Period, Time, Left, Right, NMessage);
        for (int j = 0; j < NMessage; j++)
        {
            InputFile >> SystemTask[i].OutMessage[j];
        }
    }
    InputFile >> ModNum;
    for (int i = 0; i < MessageNum; i++)
    {
        InputFile >> Src >> Dest >> Size;
        SystemMessage.emplace_back(Src, Dest, Size);
    }
    return;
}
