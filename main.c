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

char hraciaPlocha[15] = "---------------";
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
bool vyhodeny = false;
int pripojenia = 0;
int pripraveni = 0;
bool vitaz = false;
int cicina = 12;

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
        char msg[256];
        b:

        pthread_mutex_lock(&lock);

        a:

        if (vyhodeny) {

            poziciaHracaNova = 0;
            poziciaHracaStara = 0;
            //hraciaPlocha[0] = znak;
            vyhodeny = false;
            snprintf(msg,256,"Bol si vyhodeny, tvoja pozicia je 0!");
            send(newsockfd,msg,strlen(msg)+1,0);

        } else {

            snprintf(msg,256,"Si na tahu, tvoja pozicia je: %d",poziciaHracaStara);
            send(newsockfd,msg,strlen(msg)+1,0);
            bzero(buffer,256);

        }


        printf("Na tahu je hrac %c, pozicia %d\n", znak, poziciaHracaStara);

        bzero(buffer,256);

        recv(newsockfd, buffer, 256, 0);

        if (vitaz) {

            snprintf(msg,256,"Prehral si!");
            send(newsockfd,msg,strlen(msg)+1,0);
            pthread_mutex_unlock(&lock);
            bzero(buffer,256);
            recv(newsockfd, buffer, 256, 0);
            printf("%s\n",buffer);
            printf("Koniec hry.\n");
            pthread_mutex_unlock(&lock);
            break;

        }

        if (buffer[0] == '1') {


            hod = rand() % 6+1 ;
            poziciaHracaNova = (poziciaHracaStara + hod);

            if (hraciaPlocha[poziciaHracaNova-1] != '-' && poziciaHracaNova < (sizeof (hraciaPlocha))) {

                vyhodeny = true;
                printf("Hrac %c vyhodil druheho hraca!\n",znak);

            }

            if(poziciaHracaNova > (sizeof (hraciaPlocha))) {

                poziciaHracaNova = poziciaHracaStara;
                snprintf(msg,256,"Hodil si prilis vela (%d), musis sa trafit do domceka (%d)!", hod, (sizeof (hraciaPlocha) - poziciaHracaStara));
                printf("Hrac hodil: %d\n", hod);
                send(newsockfd,msg,strlen(msg)+1,0);
                pthread_mutex_unlock(&lock);
                sleep(3);
                goto b;

            }

            if (poziciaHracaNova == (sizeof (hraciaPlocha))) {
                printf("Hrac hodil: %d\n", hod);
                snprintf(msg,256,"Vyhral si!");
                send(newsockfd,msg,strlen(msg)+1,0);
                vitaz = true;
                pthread_mutex_unlock(&lock);

            }

            if (!vitaz) {

                hraciaPlocha[poziciaHracaNova-1] = znak;
                hraciaPlocha[poziciaHracaStara-1] = '-';
                poziciaHracaStara = poziciaHracaNova;
                printf("Hrac hodil: %d\n", hod);

                if (vyhodeny) {

                    snprintf(msg,256,"Hodil si: %d a vyhodil si druheho hraca!",hod);
                    send(newsockfd,msg,strlen(msg)+1,0);
                    pthread_mutex_unlock(&lock);
                    sleep(3);

                } else {

                    snprintf(msg,256,"Hodil si: %d",hod);
                    send(newsockfd,msg,strlen(msg)+1,0);
                    pthread_mutex_unlock(&lock);
                    sleep(3);

                }

            }


        } else if (buffer[0] == '2') {

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
             snprintf(msg,256,"Zla volba, vyberaj len 1-2-3!");
             send(newsockfd,msg,strlen(msg)+1,0);
             bzero(buffer,256);
             sleep(1);
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
    serv_addr.sin_port = htons(17454);

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