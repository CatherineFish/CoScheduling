#include <iostream>

#include <string>
#include <sstream>

#include "InitialGenerator.h"


int main(int argc, char* argv[])
{
    std::stringstream arg1, arg2, arg3, arg4; 
    int arg_1, arg_2, arg_3, arg_4; 
    
    arg1 << argv[1]; 
    arg2 << argv[2]; 
    arg3 << argv[3]; 
    arg4 << argv[4]; 
    arg1 >> arg_1; 
    arg2 >> arg_2; 
    arg3 >> arg_3; 
    arg4 >> arg_4; 
    
    if (argc < 5)
    {
        std::cout << "No argc" << std::endl;
        return 0;
    }
    InitialGenerator InitGen(arg_1, arg_2, arg_3, arg_4);
    
    return 0;
}