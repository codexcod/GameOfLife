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

    srand(time(0));

    //Se crean 9 clientes para llenar el tablero
    for (size_t i = 0; i < 9; i++)
    {
        threads.push_back(thread(cliente, (rand()%5000) + 1025));
    }

    for (unsigned int i = 0; i < 9; i++)
	{
		threads[i].join();
	}
    
    return 0;
}