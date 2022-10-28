#include "header.h"
using namespace std;

// Sugerencia: El servidor va a necestiar varios threads. Recordar que peuden
// compartir variables y que por lo tanto las herramientas de sincronziacion
// como semaforos son perfectamente validas.

vector<vector<int>> matrix(3, vector<int>(3));
vector<int> socketsListos


// Servicio draw: En cada tick, imprime el mapa con el estado de cada celula 
void draw()
{
    /* TO DO*/
}

// Servicio timer: Cada cierto intervalo de tiempo publica un tick. 
//Es importante que cada tick se envie utilizando el mapa de vecinos actualizado

void timer()
{
   /* TO DO*/
}



// Thread map_creator: Agrega los nuevos nodos al mapa
void map_creator(/* TO DO*/)
{
  
    /* Registrar los lsn ports de los nuevos */
    /* TIP: Hay que esperar que los clientes manden el mensaje con el lsn port*/
    /* Varias formas de hacerlo, pselect puede resultar comodo para este caso */
    /* Crear threads podria ser demasiado overhead, pero es valido */
    /* TO DO*/

    // Avisar a las celulas que correspondan la nueva estructura de vecinos
    // TIP: Puede ser util separar el caso inicial del resto, sobre todo para
    //      facilitar luego el testeo
    if(N ==3){
        base_case_3x3(/* TO DO*/);
        return;
    }
    general_case_nxn(/* TO DO*/);

}

void server_accept_conns(int newSocket)
{
    socketsListos.push_back(socketNuevo);
    if(socketsListos.size() == 9){
        for (size_t i = 0; i < 3 ; i++)
        {
            for (size_t j = 0; j < 3; j++)
            {
                matrix[i][j] = socketsListos[i*j];
                return true
            }
        }
    }
    return false
}

int main(void)
{
    int s;
    struct sockaddr_in local;
    struct sockaddr_in remote;
    vector <thread> threads;

    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("socket");
        exit(1);
    }

    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = INADDR_ANY;


    int localLink = bind(s, (struct sockaddr *)&local, sizeof(local);
    if (localLink < 0) {
        perror("bind");
        exit(1);
    }

    int listenMode = listen(s, 10);
    if (listenMode == -1) {
        perror("listen");
        exit(1);
    }

    int t = sizeof(remote);
    int socket;
    for(;;){
        socket = accept(s, (struct sockaddr*) &remote, (socklen_t*) &t);
        if(socket == -1) {
            perror("aceptando la conexión entrante");
            exit(1);
        }
        threads.push_back(thread(connection_handler, socket));
        if (server_accept_conns(socket))
        {
            request req;
            strcpy(req.msg, "Todos conectados amiguitos");
            strcpy(req.type, "Hola \n");
            broadcast(matrix,&req);
        }
    }

    for (unsigned int i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }
    
    close(s);

    return 0;
}

