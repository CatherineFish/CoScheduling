#include "MainClasses.h"
#include "AlgorithmClasses.h"

int main(int argc, char* argv[])
{
    System CurSystem(argv[1]);
    InitAlgorithm CurInitAlg(&CurSystem);
    CurSystem.PrintSystem();
    CurInitAlg.PrintTaskGraph();
    return 0;
}