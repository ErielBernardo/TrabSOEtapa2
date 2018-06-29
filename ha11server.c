#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define maxConnections 1
#define sizeBUFFER 16

void *f_thread(int*);
int create_socket(void);

//Pipes control functions
char *pipe_read(void);
void pipe_write(char *buffer);
void pipe_create(void);
void pipe_destroyer(void);

//Declarações Mutex
pthread_mutex_t mWrite;
pthread_mutex_t mRemove;

pthread_t aThreads[maxConnections];

int main(int argc , char *argv[])
{
    pipe_create();

    char *checker = NULL;
    pid_t   childpid; // PID of the child process of fork
    char buffer[sizeBUFFER];

    // Creating fork
    if((childpid = fork()) == -1)
    {
        perror("fork");
        exit(1);
    }

    if(childpid = 0) // Child process takes care of reading and wirting on disk
    {
        printf(" estou no inicio de child\n");

        while(1)
        {
            bzero(buffer,sizeBUFFER);
            printf(" estou em child antes de pipe_read()\n");
            strcpy(buffer, pipe_read());

            checker = strstr(buffer, "exit");
            if(checker == buffer)
            {
                printf("Child process ended!\n");
                break;
            }

            /* INSERIR
             * PARTE
             * DA
             * ESCRITA
             * EM
             * DISCO
             */

            printf("ainda estou em child no fim do while\n");
        }
        pipe_destroyer();
        exit(0);
    }

    // Parent process takes care of communication with client
    //pipe_write("TESTE PIPE");

    int i, status, base_socket;
    pthread_mutex_init(&mWrite,0);
    pthread_mutex_init(&mRemove,0);

    base_socket = create_socket();
    for (i=0; i < maxConnections; i++)
    {
        status = pthread_create(&aThreads[i],0,f_thread,(void *)base_socket);
        if(status != 0)
        {
            printf("Erro ao criar thread.");
            exit(-1);
        }
    }

    printf("ANIDA estou em PARENT\n - Finalizando threads e pipe\n");
    pipe_destroyer();
    for(i=0; i<maxConnections;i++)
    {
        pthread_join(aThreads[i],0);
    }

    printf("FIM de PARENT\n");
    exit(0);
}

int create_socket()
{
    int i;

    //Declarações socket
    int socket_desc , new_socket , c, portn;
    struct sockaddr_in server , client;
    char buffer[256];
    portn = 9000;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        perror("Could not create socket\n");
        exit(-1);
    }
    printf("Socket created\n");

    bzero((char *) &server, sizeof (server));

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portn);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Bind failed");
        close(socket_desc);
        exit(-1);
    }
    printf("Bind done\n");

    //Listen
    listen(socket_desc , maxConnections);

    printf("Socket created and listening...\n");

    return socket_desc;
}

void *f_thread(int *arg)
{
    int base_sd = (int) arg;
    int i,c, new_socket;
    char buffer[sizeBUFFER];
    char string[4096];
    struct sockaddr_in client;

    printf("New thread. TID = %d!\n",(int) pthread_self());

    //Accept incoming connection
    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);
    new_socket = accept(base_sd, (struct sockaddr *)&client, (socklen_t*)&c);
    if (new_socket<0)
    {
        perror("Connection rejected");
        close(base_sd);
        exit(-1);
    }
    printf("Connection accepted\n");

    while(1)
    {
        char *checker = NULL;
        bzero(buffer,sizeBUFFER);

        //Read the message from socket
        if (read(new_socket,buffer,sizeBUFFER-1) < 0)
        {
            perror("ERROR reading from socket");
            close(base_sd);
            exit(-1);
        }

        printf("\nMensagem recebida do client: %s\n", buffer);

        printf("ANIDA estou em PARENT - ANTES de 'pipe_write(buffer);'\n");
        pipe_write(buffer);
        printf("ANIDA estou em PARENT - DEPOIS de 'pipe_write(buffer);'\n");

        checker = strstr(buffer, "exit");
        if(checker == buffer)
        {
            printf("Connection with this thread ended!\n");
            write(new_socket,"Connection Closed",2048);
            break;
        }

        if (write(new_socket,"Retorno",2048) < 0)
        {
            perror("ERROR writing to socket");
            close(base_sd);
            exit(-1);
        }
    }

    close(new_socket);
    pthread_exit(0);
}

char *pipe_read(void)
{
    int fd = -1;
    char buffer[sizeBUFFER];

    // FIFO file path
    char * myfifo = "/tmp/myfifo";

    // Open FIFO for Read only
    fd = open(myfifo, O_RDONLY);
    if(fd == -1)
    {
        perror("FIFO read error");
        return NULL;
    }

    // Read from FIFO and close it
    if((read(fd, buffer, sizeBUFFER)) < 0)
    {
        perror("FIFO read error");
        return;
    }
    close(fd);

    // Print the read message
    printf("Menssagem recebida pelo pipe_read: %s\n", buffer);

    return buffer;
}

void pipe_write(char *buffer)
{
    printf("\n Em pipe_write()\n   buffer = %s\n", buffer);
    int fd = -1;

    // FIFO file path
    char * myfifo = "/tmp/myfifo";

    // Open FIFO for write only
    fd = open(myfifo, O_WRONLY);
    if(fd == -1)
    {
        perror("FIFO write error");
        return;
    }

    // Write the input buffer on FIFO and close it
    if((write(fd, buffer, sizeBUFFER)) < 0)
    {
        perror("FIFO write error");
        return;
    }
    close(fd);

    // Print the write message
    printf("Mennsagem ENVIADA pelo pipe_write: %s\n", buffer);

    return;
}


void pipe_create(void)
{
    // FIFO file path
    char * myfifo = "/tmp/myfifo";

    // Creating the named file(FIFO) (named pipe)
    // mkfifo(<pathname>, <permission>)
    mkfifo(myfifo, 0666);
}

void pipe_destroyer(void)
{
    // FIFO file path
    char * myfifo = "/tmp/myfifo";

    // Remove FIFO
    int unlink(const char * myfifo);
}
