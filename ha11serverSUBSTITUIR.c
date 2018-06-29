#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#define maxConnections 1

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
void check_command(char *, char *);
FILE* create_filesystem(void);
void create_file(FILE* pFile, char* content, char* filename);

//Declarações Mutex
pthread_mutex_t mWrite;
pthread_mutex_t mRemove;
FILE* pFile;

pthread_t aThreads[maxConnections];
 
int main(int argc , char *argv[])
{
    int i, status, base_socket;
    pthread_mutex_init(&mWrite,0);
    pthread_mutex_init(&mRemove,0);

    pFile = create_filesystem();
    
    base_socket = create_socket();
    for (i=0; i < maxConnections; i++)
    {
        status = pthread_create(&aThreads[i],0,f_thread,(void *)base_socket);
        
        if(status != 0)
        {
            printf("Error: Thread could not be created.");
            exit(-1);
        }
        
    }
    

    for(i=0; i<maxConnections;i++)
    {
        pthread_join(aThreads[i],0);
    } 
    
    fclose(pFile);
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
        
        check_command(buffer, string);

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

 void check_command(char *buffer, char *string)
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
  meta.blockSize = 8;
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
    char contenti[32] = "Conteudo de Teste";

    //strcpy(newNode.name,"teste.txt");
    strcpy(newNode.name,filename);
    newNode.size = sizeof(content);
    newNode.cTime = time(NULL);
    newNode.pBlocs = meta.fsSize - meta.freeSpace;

    meta.freeSpace = meta.freeSpace - ceil(newNode.size/8);

    fseek ( pFile , meta.pInodes, SEEK_SET );
    fwrite (&newNode, sizeof(newNode), 1, pFile);

    meta.pInodes = meta.pInodes + sizeof(newNode);

    fseek ( pFile , newNode.pBlocs, SEEK_SET );
    fwrite (&contenti, sizeof(contenti), 1, pFile);
 }
