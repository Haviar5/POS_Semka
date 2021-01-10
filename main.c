#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "pthread.h"
#include <arpa/inet.h>
#include <stdbool.h>

#define VELKOST_PLOCHY 40

char hraciaPlocha[VELKOST_PLOCHY];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
bool vyhodeny = false;
int pripojenia = 0;
int pripraveni = 0;
bool vitaz = false;

typedef struct hrac {

    int poziciaHracaStara;
    int poziciaHracaNova;
    bool maPanacika;
    int figurkyVCieli;
    int socket;


} HRAC;

void specialVyhodenie(HRAC* hrac, char plocha[], int hod) {

    char msg[256];
    plocha[hrac->poziciaHracaNova - 1] = '-';
    plocha[hrac->poziciaHracaStara - 1] = '-';
    hrac->poziciaHracaNova = 0;
    hrac->poziciaHracaStara = 0;
    hrac->maPanacika = false;

    snprintf(msg,256,"Hodil si: %d, skocil si na # a spadol si do jamy! Bohuzial tvoj panacik to neprezil.",hod);
    send(hrac->socket,msg,strlen(msg)+1,0);
    pthread_mutex_unlock(&lock);
    sleep(3);

}

void specialPrekazka(HRAC* hrac, int hod) {

    char msg[256];
    if (hod == 6) {

        snprintf(msg,256,"Hodil si: %d, a narazil by si do steny! Skus sa priblizit a preskocit stenu!",hod);

    } else {

        snprintf(msg,256,"Hodil si: %d, a narazil by si do steny! Musis sa poriadne rozbehnut a stenu preskocit!",hod);

    }

    hrac->poziciaHracaNova = hrac->poziciaHracaStara;

    send(hrac->socket,msg,strlen(msg)+1,0);
    pthread_mutex_unlock(&lock);
    sleep(3);

}

void naplnPlochu(char znak, char plocha[]) {

    for (int i = 0; i < VELKOST_PLOCHY; ++i) {
        plocha[i] = znak;
    }

}

void rozhodSpecialne(int kolko1, int kolko2, char pole[]) {

    int nahodna, upper, lower;
    for (int i = 0; i < kolko1; ++i) {
        lower = 6;
        upper = VELKOST_PLOCHY - (lower + 2) ;

        nahodna = rand() % upper + lower;
        printf("%d\n", nahodna);

        if (pole[nahodna] == '|') {

            i--;

        } else {

            pole[nahodna] = '|';

        }

    }

    for (int j = 0; j < kolko2; ++j) {

        upper = VELKOST_PLOCHY;
        lower = 6;
        upper -= (lower + 2);
        //printf("%d\n", upper);
        //nahodna = rand() % (VELKOST_PLOCHY-8) + 7 ;
        nahodna = rand() % upper + lower;
        printf("%d\n", nahodna);

        if (pole[nahodna] == '#' || pole[nahodna] == '|') {

            j--;

        } else {

            pole[nahodna] = '#';

        }

    }

}

void* nacuvaj(void* args) {

    int hod = 0;
    int newsockfd = *((int*)args);
    HRAC hrac = {0,0,false, 0, newsockfd};
    char buffer[256];

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

            hrac.poziciaHracaNova = 0;
            hrac.poziciaHracaStara = 0;
            vyhodeny = false;
            snprintf(msg,256,"Bol si vyhodeny, tvoja pozicia je 0!");
            send(newsockfd,msg,strlen(msg)+1,0);

        } else {

            snprintf(msg,256,"Si na tahu, tvoja pozicia je: %d",hrac.poziciaHracaStara);
            send(newsockfd,msg,strlen(msg)+1,0);
            bzero(buffer,256);

        }

        printf("Na tahu je hrac %c, pozicia %d\n", znak, hrac.poziciaHracaStara);

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

            if (hrac.poziciaHracaNova == 0) {

                hrac.maPanacika = false;

                if (hod == 6 || hod == 1 || hod == 3) {

                    hrac.maPanacika = true;

                } else {

                    printf("Hrac hodil: %d\n", hod);
                    snprintf(msg,256,"Hodil si zle (%d), na polozenie panacika na plochu potrebujes hodit 1 ,3 alebo 6!", hod);
                    send(newsockfd,msg,strlen(msg)+1,0);
                    pthread_mutex_unlock(&lock);
                    sleep(3);

                }

            }

            if (hrac.maPanacika) {

                hrac.poziciaHracaNova = (hrac.poziciaHracaStara + hod);

                if (hraciaPlocha[hrac.poziciaHracaNova-1] != '-' && hrac.poziciaHracaNova < (sizeof (hraciaPlocha)) && hraciaPlocha[hrac.poziciaHracaNova-1] != '#' && hraciaPlocha[hrac.poziciaHracaNova-1] != '|') {

                    vyhodeny = true;
                    printf("Hrac %c vyhodil druheho hraca!\n",znak);

                }

                if(hrac.poziciaHracaNova > (sizeof (hraciaPlocha))) {

                    hrac.poziciaHracaNova = hrac.poziciaHracaStara;
                    snprintf(msg,256,"Hodil si prilis vela (%d), musis sa trafit do domceka (%d)!", hod, (sizeof (hraciaPlocha) - hrac.poziciaHracaStara));
                    printf("Hrac hodil: %d\n", hod);
                    send(newsockfd,msg,strlen(msg)+1,0);
                    pthread_mutex_unlock(&lock);
                    sleep(3);
                    goto b;

                }

                if (hrac.poziciaHracaNova == (sizeof (hraciaPlocha))) {

                    hrac.figurkyVCieli++;

                    if (hrac.figurkyVCieli == 2) {

                        printf("Hrac hodil: %d\n", hod);
                        snprintf(msg,256,"Vyhral si!");
                        send(newsockfd,msg,strlen(msg)+1,0);
                        vitaz = true;
                        pthread_mutex_unlock(&lock);

                    } else {

                        hraciaPlocha[hrac.poziciaHracaStara-1] = '-';
                        hrac.poziciaHracaStara = 0;
                        hrac.poziciaHracaNova = 0;

                        snprintf(msg,256,"Hodil si: %d, panacika si uspecne dopravil do ciela, ostava ti este %d!",hod, (2-hrac.figurkyVCieli));
                        send(newsockfd,msg,strlen(msg)+1,0);

                        pthread_mutex_unlock(&lock);
                        sleep(3);

                    }


                } else if (!vitaz) {


                    if (hraciaPlocha[hrac.poziciaHracaNova-1] == '#') {

                        specialVyhodenie(&hrac,hraciaPlocha, hod);


                    } else if (hraciaPlocha[hrac.poziciaHracaNova-1] == '|') {

                        specialPrekazka(&hrac, hod);

                    } else {

                        hraciaPlocha[hrac.poziciaHracaNova-1] = znak;
                        hraciaPlocha[hrac.poziciaHracaStara-1] = '-';
                        hrac.poziciaHracaStara = hrac.poziciaHracaNova;
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

    naplnPlochu('-',hraciaPlocha);
    rozhodSpecialne(10,10,hraciaPlocha);

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