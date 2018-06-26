#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void menu_descr(char *buffer);

int main(int argc , char *argv[])
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[4096];
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
        //Send some msg
        /*printf("Write a message: ");
        fgets(buffer,255,stdin);*/
        strcpy(buffer, "");
        menu_descr(buffer);

        if( send(socket_desc , buffer , 255 , 0) < 0)
        {
            perror("Send failed");
            close(socket_desc);
            return -1;
        }

        //Receive reply
        bzero(buffer,4096);
        if( recv(socket_desc, buffer , 4095 , 0) < 0)
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
    printf("\n\n\n ######### funcoes suportadas ############");
    printf("\n listar               -> 'ls -la', 'ls' ");
    printf("\n criar diretorio      -> 'mkdir arg'         onde 'arg' e o nome ou caminho completodo diretorio novo desejado");
    printf("\n criar arquivo        -> 'touch arg'         onde 'arga e o nome do arquivo desejedo");
    printf("\n copiar               -> 'cp arg0 arg1'      onde 'arg0' e o arquivo de origem, 'arg1' arquivo de destino ou camiho do diretorio");
    printf("\n remover arq ou pasta -> 'rm arg, rm -r arg' onde 'arg' e o nome do arquivo ou pasta a ser removido");
    printf("\n sair                 -> 'exit'");
    printf("\n ->>");
    fgets(buffer,255,stdin);
    printf("\n\n");
}
