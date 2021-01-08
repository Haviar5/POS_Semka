#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "pthread.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

char hraciaPlocha[10] = "----------";
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
bool vyhodeny = false;
int pripojenia = 0;
int pripraveni = 0;
bool vitaz = false;

void* nacuvaj(void* args) {

    int poziciaHracaStara = 0;
    int poziciaHracaNova = 0;
    int hod = 0;
    int newsockfd = *((int*)args);
    char buffer[256];
    char serverSprava[1];

    char znak;
    recv(newsockfd, buffer, 1, 0);
    znak = buffer[0];
    pripraveni++;

    while (pripojenia != 2 || pripraveni != 2) {

        sleep(3);
        printf("Cakam na druheho hraca... (%d)\n", pripojenia);


    }

    while(1) {
        char* msg = "Si na tahu!";
        b:

        pthread_mutex_lock(&lock);

        a:

        send(newsockfd,msg,strlen(msg)+1,0);
        bzero(buffer,256);




        if (vyhodeny) {

            poziciaHracaNova = 0;
            poziciaHracaStara = 0;
            hraciaPlocha[0] = znak;
            vyhodeny = false;

        }

        printf("Na tahu je hrac %c, pozicia %d\n", znak, poziciaHracaStara);

        bzero(buffer,256);



        recv(newsockfd, buffer, 256, 0);


        if (vitaz) {

            msg = "Hra skoncila!";
            send(newsockfd,msg,strlen(msg)+1,0);
            pthread_mutex_unlock(&lock);
            bzero(buffer,256);
            recv(newsockfd, buffer, 256, 0);
            printf("%s\n",buffer);

            printf("Koniec hry.\n");
            break;

        }

        if (buffer[0] == '1') {

            /*if (vyhodeny) {

                poziciaHracaNova = 0;
                poziciaHracaStara = 0;
                vyhodeny = false;
            }*/

            hod = rand() % 6+1 ;
            poziciaHracaNova = (poziciaHracaStara + hod);

            if (hraciaPlocha[poziciaHracaNova-1] != '-' && poziciaHracaNova < (sizeof (hraciaPlocha))) {

                vyhodeny = true;
                printf("Hrac %c vyhodil druheho hraca!\n",znak);

            }

            if(poziciaHracaNova > (sizeof (hraciaPlocha))) {

                poziciaHracaNova = poziciaHracaStara;
                msg = "Hodil si prilis vela, musis sa trafit do domceka!";
                printf("Hrac hodil: %d\n", hod);
                send(newsockfd,msg,strlen(msg)+1,0);
                pthread_mutex_unlock(&lock);
                sleep(3);
                goto b;


            }

            if (poziciaHracaNova == (sizeof (hraciaPlocha))) {
                printf("Hrac hodil: %d\n", hod);
                msg = "Vyhral si!";
                send(newsockfd,msg,strlen(msg)+1,0);
                vitaz = true;
                pthread_mutex_unlock(&lock);

                //break;

            }

            if (!vitaz) {

                hraciaPlocha[poziciaHracaNova-1] = znak;
                hraciaPlocha[poziciaHracaStara-1] = '-';
                poziciaHracaStara = poziciaHracaNova;
                printf("Hrac hodil: %d\n", hod);

                bzero(serverSprava,256);
                serverSprava[0] = hod+'0';
                send(newsockfd,serverSprava,sizeof(serverSprava),0);
                bzero(buffer,256);
                pthread_mutex_unlock(&lock);
                sleep(3);

            }



        } else if (buffer[0] == '2') {

            /*if (vyhodeny) {

                poziciaHracaNova = 0;
                poziciaHracaStara = 0;
                hraciaPlocha[poziciaHracaNova-1] = znak;

            }*/

             printf("Volba 2\n");
             send(newsockfd,hraciaPlocha,sizeof(hraciaPlocha),0);
             bzero(buffer,256);
             sleep(1);
             goto a;

         } else if (buffer[0] == '3') {


            pthread_mutex_unlock(&lock);
            printf("Pripojenie ukoncene\n");
            break;

         } else {

             printf("Zla volba\n");
             bzero(buffer,256);
             goto a;

         }

     }
    close(newsockfd);


}

int main()
{

    srand(time(0));
    int sockfd;
    struct sockaddr_in serv_addr;
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(17455);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error binding socket address");
        return 2;
    }


    int newSocket;
    socklen_t cli_len;
    struct sockaddr_in cli_addr;

    bzero((char*)&cli_addr, sizeof(cli_addr));

    listen(sockfd, 5);

    pthread_t hraci[2];
    for (int i = 0; i < 2; ++i) {

        cli_len = sizeof(cli_addr);
        printf("Server caka na pripojenie.\n");

        newSocket = accept( sockfd, (struct sockaddr*)&cli_addr, &cli_len);

        if (newSocket > 0) {

            pripojenia++;

        }

        printf("Pripojenie nadviazane od: %s:%d, pripojeni hraci: %d\n",inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), pripojenia);

        if (pthread_create(&hraci[i], NULL, nacuvaj, &newSocket) != 0) {

            printf("Nepodarilo sa vytvorit vlakno!\n");

        } else {

            printf("Vytvoril som vlakno %d!\n", i);

        }

    }

    for (int i = 0; i < 2; ++i) {
        pthread_join(hraci[i],NULL);
    }


    close(sockfd);

    return 0;
}