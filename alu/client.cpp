#include "header.h"

using namespace std;

//Socket del cliente
int mSocket;

//Lista de puertos vecinos
vector<int> vecinos;

//Estado de vida de la celda
bool estado;


// Dado el estado actual de la celula y el estado de los vecinos en una ronda
// computa el nuevo estado de la celula segun las reglas del juego
bool set_state(bool alive, const vector<request>& cl)
{
	// TO DO
}

int connect_socket(int puerto)
{
	struct sockaddr_in remote;
	int vecino;
	if ((vecino = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("creando socket");
		exit(1);
	}
	remote.sin_family = AF_INET;
	remote.sin_port = htons(puerto);
	remote.sin_addr.s_addr = INADDR_ANY;
	int s = connect(vecino, (struct sockaddr *)&remote, sizeof(remote));
	if (s == -1)
	{
		perror("conectandose");
		exit(1);
	}

	return vecino;
}

//Por cada puerto genera la conexion con sus vecinos
void conectarVecinos(vector<int> &vecinosHablar)
{
	for (int i = 0; i < vecinos.size(); ++i)
	{
		vecinosHablar.push_back(connect_socket(vecinos[i]));
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

    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (socket_fd  == -1)
	{
		perror("creando socket");
		exit(1);
	}

	remote.sin_family = AF_INET;
	remote.sin_port = htons(PORT);
	inet_pton(AF_INET, "127.0.0.1", &(remote.sin_addr));

    mSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (mSocket == -1)
	{
		perror("socket");
		exit(1);
	}

    //Se setea el puerto con el numero aleatorio enviado
	int puerto = atoi(argv[1]);
	local.sin_family = AF_INET;
	local.sin_port = htons(puerto);
	local.sin_addr.s_addr = INADDR_ANY;

    int bind = bind(mSocket, (struct sockaddr *)&local, sizeof(local))
	if (bind < 0)
	{
		perror("bind");
		exit(1);
	}

    int listenMode = listen(mSocket, 10);
	if (listenMode == -1)
	{
		perror("listen");
		exit(1);
	}

	int s = connect(socket_fd, (struct sockaddr *)&remote, sizeof(remote));
	if (s == -1)
	{
		perror("conectandose");
		exit(1);
	}

	request reqPuerto;
	strncpy(reqPuerto.type, "PUERTA", 7);
	strncpy(reqPuerto.msg, to_string(puerto).c_str(), sizeof(to_string(puerto).c_str()));
	send_request(socket_fd, &reqPuerto);
	request reqEstado;
	strncpy(reqEstado.type, "ESTADO", 14);
	strncpy(reqEstado.msg, BoolToString(estado), 2);
	send_request(socket_fd, &reqEstado);
	while (1)
	{
		int socket;
		request reqInfo;
		get_request(socket_fd, &reqInfo);
		if (strncmp(reqInfo.type, "VECINOS", 8) == 0)
		{
			separarVecinos(string(reqInfo.msg), vecinos);
			threads.push_back(thread(conectarVecinos, vecinos, ref(socketsHablar)));
			threads.push_back(thread(aceptarVecinos, local, mSocket, ref(socketsEscuchar)));
		}
		if (strncmp(reqInfo.type, "TICK", 5) == 0)
		{
			threads.push_back(thread(contestarVecinos, ref(socketsHablar), estado));
			threads.push_back(thread(escucharVecinos, ref(socketsEscuchar), ref(estado), socket_fd));
		}
	}

	for (unsigned int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}

	return 0;
}