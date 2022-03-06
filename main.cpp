#include "MainClasses.h"
#include "MainAlgorithm.h"
#include "InitAlgorithm.h"
#include "CoefClasses.h"

int main(int argc, char* argv[])
{
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