#include "header.h"

//Se llama al cliente con un numero random
void cliente(int random){
    std::string command = "./cliente ";
    command += std::to_string(random);
    system(command.c_str());
}

int main(int argc, char const *argv[])
{

    vector <thread> threads;

    srand(time(NULL));

    int max = 7000;
    int min = 1000;

    //Se crean 9 clientes para llenar el tablero
    for (size_t i = 0; i < HORIZONTAL * VERTICAL; i++)
    {
        threads.push_back(thread(cliente, (rand()% max + min)));
    }

    for (unsigned int i = 0; i < HORIZONTAL * VERTICAL; i++)
	{
		threads[i].join();
	}
    
    return 0;
}