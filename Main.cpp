#include "Chip8.h"
#include <iostream>

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cout<<"No game selected"<<std::endl;
        return -1; 
    }

    Chip8 chip8;
    chip8.LoadProgram(argv[1]);
    chip8.Run();
    return 0;
}


