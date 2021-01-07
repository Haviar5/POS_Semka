#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int main()
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    char buffer[256];



    server = gethostbyname("frios2.fri.uniza.sk");
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char*)server->h_addr,
            (char*)&serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(17455);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 3;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error connecting to socket");
        return 4;
    }
    printf("Pripojil som sa na server.\n");

    bzero(buffer, 256);
    printf("\n Pleasae enter a sign: ");
    scanf("%s", &buffer[0]);
    send(sockfd,buffer,1,0);



    while (1) {

        printf("Send a message : ");
        scanf("%s", &buffer[0]);
        send(sockfd, buffer, 256,0);

        if (strcmp(buffer, "3") == 0) {
            close(sockfd);
            printf("Odpajam sa zo servera.\n");
            exit(1);

        }
        bzero(buffer,256);
        recv(sockfd,buffer,256,0);
        printf("Sprava: %s\n",buffer);

    }


    return 0;
}