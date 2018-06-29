#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define sizeBUFFER 256

void menu_descr(char *buffer);

int main(int argc , char *argv[])
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[sizeBUFFER];
    int socket_desc, portn;
    char *checker = NULL;

    portn = 9000;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        perror("Could not create socket");
        return -1;
    }
    printf("Socket Created\n");

    //Prepare the sockaddr_in structure
    server = gethostbyname("localhost");  //change here to work outside the VM
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portn);

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0)
    {
        perror("Connect error");
        close(socket_desc);
        return -1;
    }
    printf("Connected\n");

    while(1)
    {
        char *checker = NULL;

        bzero(buffer, sizeBUFFER);
        //menu_descr(buffer);
        fgets(buffer,sizeBUFFER,stdin);

        if( send(socket_desc , buffer , sizeBUFFER , 0) < 0)
        {
            perror("Send failed");
            close(socket_desc);
            return -1;
        }

        //Receive reply
        bzero(buffer, sizeBUFFER);
        if( recv(socket_desc, buffer , sizeBUFFER , 0) < 0)
        {
            perror("Receive failed");
            close(socket_desc);
            return -1;
        }
        printf("Reply received: ");
        printf("%s\n",buffer);

        checker = strstr(buffer, "Connection Closed");
        if(checker == buffer)
        {
            break;
        }
    }
    close(socket_desc);

    return 0;
}

void menu_descr(char *buffer)
{
    printf("\n\n ######### COMANDOS ############");
    printf("\n  Para sair           -> 'exit'");
    printf("\n  Para criar arquivo  -> 'arg0' 'arg1'    |   onde 'arg0' é o nome do arquivo e 'arg1' é o conteudo  ");
    printf("\n  --->>");
    fgets(buffer,sizeBUFFER,stdin);
    printf("\n\n");
}
