#include "header.h"
using namespace std;

// Sugerencia: El servidor va a necestiar varios threads. Recordar que peuden
// compartir variables y que por lo tanto las herramientas de sincronziacion
// como semaforos son perfectamente validas.


//Lista de los sockets clientes en el tablero
vector<vector<int>> socketsClientes(3, vector<int>(3));
//Lista de los puertos clientes
vector<vector<int>> puertosClientes(3, vector<int>(3));
//Lista de los sockets antes de arrancar el juego
vector<int> socketsListos;

//Utilizamos una lista con las ubicaciones a sumar a las cordenadas de cada clientes
//para calcular la posicion de los vecinos
vector<vector<int>> vecinosPosibles{vector<int>{1, 1},
vector<int>{0, 1},
vector<int>{-1, 1},
vector<int>{1, -1},
vector<int>{1, 0},
vector<int>{-1, 0},
vector<int>{0, -1},
vector<int>{-1, -1}};



// Servicio draw: En cada tick, imprime el mapa con el estado de cada celula 
void draw()
{
    cout << "Tablero : " << endl;
    string tablero = "";
    int contadorVidas = 0;
	for (size_t i = 0; i < socketsClientes.size(); i++)
    {
		tablero+= "\n";
        for (size_t j = 0; j < socketsClientes.size(); j++)
        {
			request reqEstado;
            get_request(&reqEstado, socketsClientes[i][j]);
			tablero+= " ";
			tablero+= reqEstado.msg;
			tablero+= " ";

            if(strncmp(reqEstado.msg, "1", 2) == 0){
                contadorVidas++;
            }
        }   
    }
    if(contadorVidas > 0){
        cout << tablero << endl;
    }else{
    	cout << "Ya no quedan mas celdas vivas" << endl;
        exit(1);
    }

}

// Servicio timer: Cada cierto intervalo de tiempo publica un tick. 
//Es importante que cada tick se envie utilizando el mapa de vecinos actualizado
void timer()
{
	int contador = 0;

    system("clear");
    cout << "Comienza el juego" << endl;

	while (1)
	{
		draw();
		string tick = "Tiempo " + to_string(contador);
		char tiempo[tick.length() + 1];
		strcpy(tiempo, tick.c_str());
		request req;
		strncpy(req.msg, tiempo, sizeof(tiempo));
		strncpy(req.type, "TICK", 5);
		broadcast(socketsClientes, &req);
		contador++;
		sleep(5);
	}
}

//Se agrega un nuevo socket hasta alcanzar la cantidad maxima
void server_accept_conns(int s, sem_t& semaforo)
{
    struct sockaddr_in remote;
	int t = sizeof(remote);
	int socket;
	for (size_t i = 0; i < 3;i++)
	{
        for (size_t j = 0; i < 3;j++)
        {
            if ((socket = accept(s, (struct sockaddr *)&remote, (socklen_t *)&t)) == -1)
            {
                perror("Error aceptando cliente");
                exit(1);
            }
            cout << "New socket" + socket << endl;
            socketsListos.push_back(socket);
            sem_post(&semaforo);
        }
	}  
}

bool llenarLista(){
    if (socketsListos.size() == 9)
	{
		cout << "EL juego comenzara en 5 segundos" << endl;
		sleep(5);
		int contador = 0;
		for (size_t i = 0; i < socketsClientes.size(); i++)
		{
			for (size_t j = 0; j < socketsClientes.size(); j++)
			{
				socketsClientes[i][j] = socketsListos[contador];
				
				contador++;
			}
		}
		return true;
	}

	return false;
}


//Chequea que la posicion sea valida y la agrega al array de vecinos
void calcularUbicacionVecino(int x, int y, int i, vector<vector<int>> &vecinos)
{
        int xVecino = x + vecinosPosibles[i][0];
        int yVecino = y + vecinosPosibles[i][1];
        if (xVecino > -1 && yVecino > -1 && xVecino < socketsClientes.size() && yVecino < socketsClientes.size())
        {
            vecinos.push_back(vector<int>{xVecino, yVecino});
        }
}

//Se calculan las posiciones de los posibles vecinos
vector<vector<int>> getVecinos(int x, int y)
{
	vector<vector<int>> vecinos;
    for(size_t i = 0; i < 8; i++){
        calcularUbicacionVecino(x, y, i, vecinos);
    }
	return vecinos;
}

//Se genera el string que funcionara como mensaje para que el cliente conozca sus vecinos
string stirngVecinos(vector<vector<int>> &vecinos)
{
	string vecinosString = "";

	for (int i = 0; i < vecinos.size(); ++i)
	{
		vecinosString += "-";
		vecinosString += to_string(puertosClientes[vecinos[i][0]][vecinos[i][1]]);
	}
	return vecinosString;
}


//Por cada cliente se recolecta la informacion de sus vecinos y se envia a cada uno
void notificarClientes()
{
	for (size_t i = 0; i < socketsClientes.size(); i++)
	{
		for (size_t j = 0; j < socketsClientes.size(); j++)
		{
            //Se calcula las posiciones de los vecinos de cada casilla
			vector<vector<int>> vecinos = getVecinos(i, j);
            //Se genera un string con los puertos de los clientes vecinos 
			string vecinosString = stirngVecinos(vecinos);
            //Se envia la info a cada cliente
			request req;
			strncpy(req.type, "VECINOS", 8);
			strncpy(req.msg, vecinosString.c_str(), MENSAJE_MAXIMO);
			send_request(socketsClientes[i][j], &req);
		}
	}
}

int main(void)
{
    int s;
    struct sockaddr_in local;
    struct sockaddr_in remote;
    vector <thread> threads;

    sem_t semaforoConexiones;
	sem_init(&semaforoConexiones, 0, 0);

    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("Error socket server");
        exit(1);
    }

    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = INADDR_ANY;


    int localLink = bind(s, (struct sockaddr *)&local, sizeof(local));
    if (localLink < 0) {
        perror("Error bind server");
        exit(1);
    }

    int listenMode = listen(s, 10);
    if (listenMode == -1) {
        perror("Error listen server");
        exit(1);
    }

    cout << "Comenzo la conexion con clientes" << endl;

    threads.push_back(thread(server_accept_conns, s, ref(semaforoConexiones)));

    for (size_t i = 0;  i < 9; i++)
	{
		sem_wait(&semaforoConexiones);
	}

    cout << "Termino la conexion con clientes" << endl;

    if (llenarLista())
    {
        for (size_t i = 0; i < socketsClientes.size(); i++)
        {
            for (size_t j = 0; j < socketsClientes.size(); j++)
            {
                request requestCliente;
                get_request(&requestCliente, socketsClientes[i][j]);
                char puerto[sizeof(requestCliente.msg)];
                strncpy(puerto, requestCliente.msg, sizeof(requestCliente.msg));
                puertosClientes[i][j] = atoi(puerto);
                cout << "New Port: " << endl;
                cout << puerto << endl;
            }
        }
        notificarClientes();
        threads.push_back(thread(timer));
    }


    for (unsigned int i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }
    
    close(s);

    return 0;
}

