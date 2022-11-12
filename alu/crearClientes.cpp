#include "header.h"
#include <unordered_set>

//Se llama al cliente con un numero random
void cliente(int random){
    std::string command = "./cliente ";
    command += std::to_string(random);
    system(command.c_str());
}

int main(int argc, char const *argv[])
{

    srand(time(NULL));

    //Seteamos maximos y minimos para el random de puertos
    int max = 7000;
    int min = 1000;

    int pid = -1;

    //Utilizamos un HashSet para no repetir los puertos ya que los puertos de los clientes tienen que ser unicos
    unordered_set <int> portsUsados ;

    //Se crean clientes para llenar el tablero
    for (size_t i = 0; i < HORIZONTAL * VERTICAL; i++)
    {
        pid = -1;
        int random = rand()% max + min; 

        //Buscamos el puerto en el set y si ya esta hacemos otro random
        while (portsUsados.find(random) != portsUsados.end())
        {
            random = rand()% max + min; 
        }
        portsUsados.insert(random);

        //Si tira error porque el puerto esta usado es que algun otro proceso esta usando ese puerto 
        //Creamos un proceso para cada cliente para que no compartan memoria y simulen clientes reales
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