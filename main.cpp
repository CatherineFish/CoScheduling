#include "MainClasses.h"
#include "MainAlgorithm.h"
#include "InitAlgorithm.h"
#include "CoefClasses.h"
#include "LimitedSearch.h"

#include <string>
#include <sstream>


int main(int argc, char* argv[])
{
    if (argc < 5)
    {
        std::cout << "No input and output files" << std::endl;
        return 0;
    }

    std::stringstream arg1, arg2; 
    int arg_1, arg_2; 
    
    arg1 << argv[3]; 
    arg2 << argv[4]; 
    
    arg1 >> arg_1; 
    arg2 >> arg_2; 
    
    System CurSystem(argv[1]);
    InitAlgorithm CurInitAlg(&CurSystem);
    
    CurInitAlg.MainLoop(&CurSystem);
    //CurSystem.PrintSystem();
    
    MainAlgorithm CurMainAlg(&CurSystem);
    
    CurMainAlg.MainLoop(&CurSystem, arg_1, arg_2);
    //CurMainAlg.PrintJobSystem(&CurSystem);
    
    //CurSystem.PrintPC();
    //CurSystem.PrintMessages();
    CurSystem.ExportToDotFile(argv[2]);
    CurMainAlg.ResultCheck(&CurSystem);
    
    return 0;
}