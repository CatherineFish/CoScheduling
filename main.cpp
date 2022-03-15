#include "MainClasses.h"
#include "MainAlgorithm.h"
#include "InitAlgorithm.h"
#include "CoefClasses.h"
#include "LimitedSearch.h"


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "No input file" << std::endl;
        return 0;
    }
    System CurSystem(argv[1]);
    InitAlgorithm CurInitAlg(&CurSystem);
    
    CurInitAlg.MainLoop(&CurSystem);
    CurSystem.PrintSystem();
    
    MainAlgorithm CurMainAlg(&CurSystem);
    
    CurMainAlg.MainLoop(&CurSystem);
    //CurMainAlg.PrintJobSystem(&CurSystem);
    
    CurSystem.PrintPC();
    CurSystem.PrintMessages();
    return 0;
}