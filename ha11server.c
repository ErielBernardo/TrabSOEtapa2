#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <math.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define maxConnections 1
#define sizeBUFFER 256

struct inode
{
    char name[20];
    int size;
    time_t cTime;
    int pBlocs;
};

struct metadata
{
    int fsSize;
    int pContent;
    int pInodes;
    int blockSize;
    int freeSpace;
};

struct metadata meta;


void *f_thread(int*);
int create_socket(void);
void check_command(char *);
FILE* create_filesystem(void);
void create_file(FILE* pFile, char* content, char* filename);

//Declarações Mutex
pthread_mutex_t mWrite;
FILE* pFile;

pthread_t aThreads[maxConnections];

int main(int argc, char *argv[])
{
    // FIFO file path
    char * fsFIFO = "/tmp/fsFIFO";
    char * svFIFO = "/tmp/svFIFO";
    // Remove FIFO
    int unlink(const char * fsFIFO);
    int unlink(const char * svFIFO);
    // Creating the named file(FIFO) (named pipe)
    // mkfifo(<pathname>, <permission>)
    mkfifo(fsFIFO, 0666);
    mkfifo(svFIFO, 0666);

    char buffer[sizeBUFFER];

    int i, status, base_socket;
    pthread_mutex_init(&mWrite,0);

    pid_t   childpid; // PID of the child process of fork
    // Creating fork
    if((childpid = fork()) == -1)
    {
        perror("fork");
        exit(1);
    }
    if(childpid == 0) // Child process takes care of reading and writing on disk
    {
        pFile = create_filesystem();
        printf("File system has been created\n");
        int fsvFIFO;
        int ffsFIFO;
        char string[sizeBUFFER];
        while(1)
        {
            char *checker = NULL;
            bzero(buffer,sizeBUFFER);
            bzero(string,sizeBUFFER);

            fsvFIFO = -1;
            // Open FIFO for Read only
            fsvFIFO = open(svFIFO, O_RDONLY);
            if(fsvFIFO == -1)
            {
                perror("FIFO read error");
                break;
            }
            // Read from FIFO and close it
            if((read(fsvFIFO, buffer, sizeBUFFER)) < 0)
            {
                perror("FIFO read error");
                break;
            }

            checker = strstr(buffer, "exit");
            if(checker == buffer)
            {
                //break;
            }
            printf("\n\n mensagem recebida pelo filesystem =  %s", buffer);

            check_command(buffer); /* MAKE COMMAND */

            /* RETURN OF COMMAND */
            ffsFIFO = -1;
            // Open FIFO for Read only
            ffsFIFO = open(fsFIFO, O_WRONLY);
            if(ffsFIFO == -1)
            {
                perror("FIFO write error");
                break;
            }
            // Write from FIFO
            if((write(ffsFIFO, "Command has been done successfully", 40) < 0))
            {
                perror("FIFO write error");
                break;
            }
            printf("Ainda estou em child no fim do while\n");
        }
        printf("*** FIM de CHILD PROCESS ****\n");
        close(fsvFIFO);
        close(ffsFIFO);
        exit(0);
    }

    // Parent process takes care of communication with client

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

    for(i=0; i<maxConnections; i++)
    {
        pthread_join(aThreads[i],0);
    }

    printf("*** FIM de PARENT PROCESS ***\n");
    exit(0);
}

int create_socket()
{
    int i;

    //Declarações socket
    int socket_desc, new_socket, c, portn;
    struct sockaddr_in server, client;
    char buffer[256];
    portn = 9000;

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
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
    if( bind(socket_desc,(struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind failed");
        close(socket_desc);
        exit(-1);
    }
    printf("Bind done\n");

    //Listen
    listen(socket_desc, maxConnections);

    printf("Socket created and listening...\n");

    return socket_desc;
}

void *f_thread(int *arg)
{
    int base_sd = (int) arg;
    int i,c, new_socket;
    char buffer[sizeBUFFER];
    char string[sizeBUFFER];
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

    int fsvFIFO;
    int ffsFIFO;
    while(1)
    {
        // FIFO file path
        char * svFIFO = "/tmp/svFIFO";

        fsvFIFO = -1;

        char *checker = NULL;
        bzero(buffer, sizeBUFFER);
        bzero(string, sizeBUFFER);

        //Read the message from socket
        if (read(new_socket,buffer,sizeBUFFER) < 0)
        {
            perror("ERROR reading from socket");
            close(base_sd);
            exit(-1);
        }
        printf("\n\n\nMensagem recebida do client: %s\n", buffer);

        // Open FIFO for write only
        fsvFIFO = open(svFIFO, O_WRONLY);
        if(fsvFIFO == -1)
        {
            perror("FIFO write error");
            break;
        }
        // Write the input buffer on FIFO
        if((write(fsvFIFO, buffer, sizeBUFFER)) < 0)
        {
            perror("FIFO write error");
            break;
        }

        checker = strstr(buffer, "exit");
        if(checker == buffer)
        {
            printf("Connection with this thread ended!\n");
            write(new_socket, "Connection Closed", 17);
            break;
        }

        /* READING RETURN MESSAGE */
        // FIFO file path
        char * fsFIFO = "/tmp/fsFIFO";
        ffsFIFO = -1;
        // Open FIFO for Read only
        ffsFIFO = open(fsFIFO, O_RDONLY);
        if(ffsFIFO == -1)
        {
            perror("FIFO read error");
            break;
        }
        // Read from FIFO and close it
        if((read(ffsFIFO, string, sizeBUFFER)) < 0)
        {
            perror("FIFO read error");
            break;
        }

        printf("ANIDA estou em PARENT - DEPOIS de 'pipe_write(buffer);'\n");

        if (write(new_socket, string, sizeBUFFER) < 0)
        {
            perror("ERROR writing to socket");
            close(base_sd);
            exit(-1);
        }
    }
    close(new_socket);
    close(fsvFIFO);
    pthread_exit(0);
}

void check_command(char *buffer)
{
    char *aux;
    char fname[20];
    char content[32];

    aux = strtok (buffer," ");
    strcpy(fname, aux);


    aux = strtok (NULL," ");
    strcpy(content, aux);

    create_file(pFile,content,fname);
}

FILE* create_filesystem()
{

    meta.fsSize = 1024;
    meta.pContent = 400;
    meta.pInodes = 24;
    meta.blockSize = 1;
    meta.freeSpace = 624;

    FILE * pFile;
    pFile = fopen ("myfile.bin","wb");
    if (pFile!=NULL)
    {
        fwrite(&meta, sizeof(struct metadata), 1, pFile);
    }
    return pFile;
}

void create_file(FILE* pFile, char* content, char* filename)
{
    struct inode newNode;

    printf("%s\n",content);
    printf("%s\n",filename);
    char contenti[32];
    strcpy(contenti,content);

    //strcpy(newNode.name,"teste.txt");
    strcpy(newNode.name,filename);
    newNode.size = (int) strlen(contenti);
    newNode.cTime = time(NULL);
    newNode.pBlocs = meta.fsSize - meta.freeSpace;

    printf("%lu\n",sizeof(contenti));

    meta.freeSpace = meta.freeSpace - ceil(newNode.size);

    pthread_mutex_lock(&mWrite);

    fseek ( pFile, meta.pInodes, SEEK_SET );
    fwrite (&newNode, sizeof(newNode), 1, pFile);

    meta.pInodes = meta.pInodes + sizeof(newNode);

    fseek ( pFile, newNode.pBlocs, SEEK_SET );
    fwrite (&contenti, strlen(contenti), 1, pFile);

    fseek ( pFile, 0, SEEK_SET );
    fwrite(&meta, sizeof(struct metadata), 1, pFile);
    pthread_mutex_unlock(&mWrite);
}
