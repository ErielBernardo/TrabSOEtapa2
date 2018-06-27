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

#define maxConnections 2

void *f_thread(int*);
int create_socket(void);

//Declarações Mutex
pthread_mutex_t mWrite;
pthread_mutex_t mRemove;

pthread_t aThreads[maxConnections];

int main(int argc , char *argv[])
{
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

    for(i=0; i<maxConnections;i++)
    {
        pthread_join(aThreads[i],0);
    }

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
    char buffer[256];
    char string[4096];
    struct sockaddr_in client;
    pid_t   childpid;

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

    /**/

    if((childpid = fork()) == -1)
    {
        perror("fork");
        exit(1);
    }

    if(childpid == 0)
    {
        /* Child process closes up input side of pipe */
        close(fd[0]);

        /* Send "string" through the output side of pipe */
        write(fd[1], string, (strlen(string)+1));
        exit(0);
    }
    else
    {
        /* Parent process closes up output side of pipe */
        close(fd[1]);

        /* Read in a string from the pipe */
        nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
        printf("Received string: %s", readbuffer);
    }

    /**/

    while(1)
    {
        char *checker = NULL;
        bzero(buffer,256);
        bzero(string,2048);
        //Read the message from socket
        if (read(new_socket,buffer,255) < 0)
        {
        perror("ERROR reading from socket");
        close(base_sd);
        exit(-1);
        }

        checker = strstr(buffer, "exit");
        if(checker == buffer)
        {
            printf("Connection with this thread ended!\n");
            write(new_socket,"Connection Closed",2048);
            break;
        }

        if (write(new_socket,string,2048) < 0)
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
    /*// Destructing  pipe
    int unlink(const char * myfifo);
    */

    char buffer[16];

    // Creating the named file(FIFO) (named pipe)
    // mkfifo(<pathname>, <permission>)
    mkfifo(myfifo, 0666);

    // Open FIFO for Read only
    fd = open(myfifo, O_RDONLY);

    // Read from FIFO
    read(fd, arr1, sizeof(buffer));

    // Print the read message
    printf("Menssagem recebida pelo pipe_read: %s\n", buffer);
    close(fd);

    return buffer;
}

void pipe_write(char *buffer)
{
        // Destructing  pipe
        int unlink(const char * myfifo);

        // Creating the named file(FIFO) (named pipe)
        // mkfifo(<pathname>, <permission>)
        mkfifo(myfifo, 0666);

        // Open FIFO for write only
        fd = open(myfifo, O_WRONLY);

        // Write the input arr2ing on FIFO
        // and close it
        write(fd, buffer, strlen(buffer)+1);
        close(fd);

        // Print the read message
        printf("Mennsagem enviada pelo pipe_write: %s\n", arr1);
        close(fd);
}
