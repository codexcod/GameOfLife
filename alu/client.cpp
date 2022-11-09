#include "header.h"

using namespace std;

//Socket del cliente
int mSocket;

//Lista de puertos vecinos
vector<int> vecinos;

//Estado de vida de la celda
bool estado;

//Puerto del cliente
int puerto;


// Dado el estado actual de la celula y el estado de los vecinos en una ronda
// computa el nuevo estado de la celula segun las reglas del juego
void set_state(int vecinosVivos)
{
	if (vecinosVivos > 3) {
		estado = false;
	}
	if (vecinosVivos < 2) {
		estado = false;
	}
	if (vecinosVivos == 3) {
		estado = true;
	}
	if (vecinosVivos == 2 && estado) {
		estado = true;
	}
}

//Notificamos al server nuestro nuevo estado
void notificarServer(int socketServer)
{
	request reqEstado;
	strncpy(reqEstado.type, "ESTADO", 7);
	strncpy(reqEstado.msg, estado ? "1" : "0", 2);
	send_request(socketServer, &reqEstado);
}

//Aceptamos las conexiones entrantes de los vecinos
void aceptarConexiones(sockaddr_in addr, vector<int> &socketsEscuchar)
{
	int t = sizeof(addr);
	for (;;)
	{
		int socket = accept(mSocket, (struct sockaddr *)&addr, (socklen_t *)&t);
		if (socket == -1)
		{
			perror("Error aceptando vecino");
			exit(1);
		}
		socketsEscuchar.push_back(socket);
	}
}

//Conectarse a un vecino
int connect_socket(int puerto)
{
	struct sockaddr_in remote;
	int vecino;
	if ((vecino = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Error creando socket vecino");
		exit(1);
	}
	remote.sin_family = AF_INET;
	remote.sin_port = htons(puerto);
	remote.sin_addr.s_addr = INADDR_ANY;
	int s = connect(vecino, (struct sockaddr *)&remote, sizeof(remote));
	if (s == -1)
	{
		perror("Error conectandose vecino");
		exit(1);
	}

	return vecino;
}

//Cada vez que hay un tick se escucha el estado de los vecinos para setea el nuevo estado de la celula
void escucharVecinos(vector<int> &socketsVecinos, int serverSocket)
{
	int vecinosVivos = 0;
	for (int i = 0; i < socketsVecinos.size(); ++i)
	{
		request req;
        get_request(&req, socketsVecinos[i]);
		if (strncmp(req.msg, "1", 2) == 0)
        {
            vecinosVivos++;
        }
    }

	set_state(vecinosVivos);
	notificarServer(serverSocket);
}

//El cliente envia su estado a sus vecinos 
void notificarVecinos(vector<int> &socketsHablar)
{
	for (int i = 0; i < socketsHablar.size(); ++i)
	{
		request reqEstado;
        strncpy(reqEstado.type, "ESTADO", 7);
        strncpy(reqEstado.msg, estado ? "1" : "0", 2);
        send_request(socketsHablar[i], &reqEstado);
	}
}

//Por cada puerto genera la conexion con sus vecinos
void conectarVecinos(vector<int> &socketsHablar)
{
	for (int i = 0; i < vecinos.size(); ++i)
	{
		socketsHablar.push_back(connect_socket(vecinos[i]));
	}
}

void getPuertosVecinos(string puertosVecinos, vector<int> &puertos)
{
	const char separador = '-';
	stringstream ss(puertosVecinos);

	string s;
	while (std::getline(ss, s, separador))
	{
		if (s != "")
		{
			puertos.push_back(atoi(s.c_str()));
		}
	}
}

int main(int argc, char* argv[]){
    int pid;

    struct sockaddr_in remote;
	struct sockaddr_in local;
	struct hostent *hp;
	struct in_addr addr;
	char buf[MENSAJE_MAXIMO];

    //Recibe un numero aleatorio por el cual  se decide si la celda esta viva o muerta
	estado = atoi(argv[1]) % 2 == 0;

	vector<thread> threads;

	vector<int> socketsHablar;
	vector<int> socketsEscuchar;


    mSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (mSocket == -1)
	{
		perror("Error creando socket cliente");
		exit(1);
	}

	puerto = atoi(argv[1]);
	local.sin_family = AF_INET;
	local.sin_port = htons(puerto);
	local.sin_addr.s_addr = INADDR_ANY;

    int localLink = bind(mSocket, (struct sockaddr *)&local, sizeof(local));
	if (localLink < 0)
	{
		perror("Error bind cliente");
		exit(1);
	}

    int listenMode = listen(mSocket, 10);
	if (listenMode == -1)
	{
		perror("Error listen cliente");
		exit(1);
	}

	int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (socket_fd  == -1)
	{
		perror("Error creando socket server");
		exit(1);
	}

	remote.sin_family = AF_INET;
	remote.sin_port = htons(PORT);
	inet_pton(AF_INET, "127.0.0.1", &(remote.sin_addr));

	int s = connect(socket_fd, (struct sockaddr *)&remote, sizeof(remote));
	if (s == -1)
	{
		perror("Error conectandose server");
		exit(1);
	}

	//Envia su puerto al servidor
	request reqPuerto;
	strncpy(reqPuerto.type, "PORT", 5);
	strncpy(reqPuerto.msg, to_string(puerto).c_str(),5);
	send_request(socket_fd, &reqPuerto);
	cout << puerto << endl;

    //Envia su estado al servidor
	request reqEstado;
	strncpy(reqEstado.type, "ESTADO", 7);
	strncpy(reqEstado.msg, estado ? "1" : "0", 2);
	send_request(socket_fd, &reqEstado);

	while (1)
	{
		int socket;
		request reqInfo;
		get_request(&reqInfo, socket_fd);
        //Esta llamada deberia llamarse una sola vez cuando el server notifique 
        //los puertos vecinos del cliente
		if (strncmp(reqInfo.type, "VECINOS", 8) == 0)
		{
			vecinos.clear();
			socketsHablar.clear();
			socketsEscuchar.clear();	
			estado = atoi(argv[1]) % 2 == 0;
            //Separa el string que recibe el servidor para conocer los puertos vecinos
			getPuertosVecinos(string(reqInfo.msg), vecinos);
			threads.push_back(thread(conectarVecinos, ref(socketsHablar)));
			threads.push_back(thread(aceptarConexiones, local, ref(socketsEscuchar)));

		}

        //Cada 5 sec el servidor envia un tick en el cual el cliente actualiza su estado
		if (strncmp(reqInfo.type, "TICK", 5) == 0)
		{
			threads.push_back(thread(notificarVecinos, ref(socketsHablar)));
			threads.push_back(thread(escucharVecinos, ref(socketsEscuchar), socket_fd));
		}
	}

	for (unsigned int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}

	return 0;
}