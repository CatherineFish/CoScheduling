#include "MainClasses.h"
#include "AlgorithmClasses.h"
#include "CoefClasses.h"

int main(int argc, char* argv[])
{
    System CurSystem(argv[1]);
    InitAlgorithm CurInitAlg(&CurSystem);
    //CurSystem.PrintSystem();
    //CurInitAlg.PrintTaskGraph();
    //CurInitAlg.FindAllPath(&CurSystem);
    MainAlgorithm CurMainAlg(&CurSystem);
    CurMainAlg.PrintJobSystem();
    CurMainAlg.MainLoop(&CurSystem);
    return 0;
}