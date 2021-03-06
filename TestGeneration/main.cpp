#include <iostream>

#include <string>
#include <sstream>
#include "InitialGenerator.h"
#include "../MainClasses.h"



int main(int argc, char* argv[])
{
    std::stringstream arg1, arg2, arg3, arg4, arg5, arg6; 
    int arg_1, arg_2, arg_3, arg_4, arg_5, arg_6; 
    
    arg1 << argv[1]; 
    arg2 << argv[2]; 
    arg3 << argv[3]; 
    arg4 << argv[4]; 
    arg5 << argv[5]; 
    arg6 << argv[6]; 
    
    arg1 >> arg_1; 
    arg2 >> arg_2; 
    arg3 >> arg_3; 
    arg4 >> arg_4; 
    arg5 >> arg_5; 
    arg6 >> arg_6; 
    
    if (argc < 8)
    {
        std::cout << "No argc" << std::endl;
        return 0;
    }
    InitialGenerator InitGen(argv[7], arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);
    
    return 0;
}