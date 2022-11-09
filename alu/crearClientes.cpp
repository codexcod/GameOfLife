#include "header.h"

//Se llama al cliente con un numero random
void cliente(int random){
    std::string command = "./cliente ";
    command += std::to_string(random);
    system(command.c_str());
}

int main(int argc, char const *argv[])
{

    srand(time(NULL));

    int max = 7000;
    int min = 1000;

    int pid = -1;

    //Se crean clientes para llenar el tablero
    for (size_t i = 0; i < HORIZONTAL * VERTICAL; i++)
    {
        pid = -1;
        int random = rand()% max + min; 
        pid = fork();
        if (pid == 0) {
            cliente(random);
            return 0;
        }

    }

    if (pid > 0)
    {
        for (size_t i = 0; i < HORIZONTAL * VERTICAL; i++) {
            wait(NULL); // Espera a clientes 
        }
        exit(0);
    }
    
    
    return 0;
}