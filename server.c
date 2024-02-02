#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>

#define PORT 4550
#define GRID_SIZE 10
#define MAX_DEBRIS 20
#define MAX_DETRITI 2 //Massimo dei detriti generati nelle funzioni
#define WINDOW_HEIGHT 10
#define DEBRIS_GENERATION_INTERVAL 2000000 // 2 secondi in microsecondi

//Struct
//informazioni sull'indirizzo e la porta di un socket
struct sockaddr_in servaddr;

//Gestione del tempo
struct timeval lastDebrisTime, currentTime;

//rappresentazione tempo in secondi
struct timeval tv;

//informazioni sull'indirizzo e la porta di un client o di un socket remoto
struct sockaddr_in clientAddr;

//Posizione navicella
struct SpaceshipPosition pos;

struct Debris {
    int x, y;
    int active;
};

struct DebrisPacket {
    struct Debris debris[MAX_DEBRIS];
};

struct Debris debris[MAX_DEBRIS];

struct SpaceshipPosition {
    int x;
};

struct DebrisPacket packet;

//Function
//Inizializza i detriti
void initDebris() {
    int count = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        if (!debris[i].active && count < MAX_DETRITI) {
            debris[i].x = rand() % GRID_SIZE;
            debris[i].y = 0;
            debris[i].active = 1;
            count++; // Incrementa il contatore per ogni detrito aggiunto
        }
    }
}

//Generazione detriti in traiettoria della navicella
void generateDebrisBasedOnSpaceship(int pos)
 {
    int count = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        if (!debris[i].active && count < MAX_DETRITI) {
            debris[i].x = pos;
            debris[i].y = 0;
            debris[i].active = 1;
            count++; // Incrementa il contatore per ogni detrito aggiunto
        }
    }
}
//Aggiornamento posizioni detriti attivi, spostandoli una volta arrivati verso il basso, in alto, in posizione randomica
void updateDebrisPositions() {
    int count = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        if (debris[i].active) {
            debris[i].y += 1;
            if (debris[i].y > WINDOW_HEIGHT && count < MAX_DEBRIS) {
                debris[i].y = 0; // Riporta il detrito in cima per simulare un flusso continuo
                debris[i].x = rand() % GRID_SIZE; // Cambia la posizione x in modo randomico
                count++;
            }
        }
    }
}

//Invio "detriti" attivi nell'array debris al client, attraverso una socket UDP, inserendoli in una struct DebrisPacket
void sendDebrisPacket(int sockfd, struct sockaddr_in *clientAddr) {

    for (int i = 0; i < GRID_SIZE; i++) {
       if (debris[i].active) {
           packet.debris[i] = debris[i];
        }
    }
    if (sendto(sockfd, &packet, sizeof(struct DebrisPacket), 0, (struct sockaddr *)clientAddr, sizeof(*clientAddr)) <0)
            {
        perror("Errore nell'invio del pacchetto dei detriti");
            }
}

int main() {
    int sockfd;
    fd_set writefds; //Per tenere traccia dei descrittori di file, dove monitoriamo eventi di scrittura, in questo caso lo spawn dei detriti in direzione della navicella
    int retval;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    //Creazione socket UDP e stampa in cso di errori
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    //Configurazione socket impostando indirizzo e porta
    memset(&servaddr, 0, sizeof(servaddr)); //inizializzazione
    servaddr.sin_family = AF_INET; //imposto famiglia indirizzi a AF_INET
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //Server in ascolto su tutte le interfaccie di rete
    servaddr.sin_port = htons(PORT); //Impostazione porta del socket

    //Associazione (binding) del socket ad un indirizzo e porta speicifica
    if (bind(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Binding fallito");
        exit(EXIT_FAILURE);
    }

    //Inizilizzazione di generazione di numeri casuali
    srand(time(NULL));


    memset(debris, 0, sizeof(debris)); //iniziallizazione array debris con zeri
    gettimeofday(&lastDebrisTime, NULL); //Otteniamo l'ora corrente
    socklen_t clientAddrLen = sizeof(clientAddr); //Utilizziamo la dimensione di clientaddr come argomento, in modo che la func sappiamo quanto spazio di memoria è stato allocato per l'indirizzo del client

    initDebris();

    // Azzera l'insieme di file descriptor e aggiunge il socket
    FD_ZERO(&writefds);
    FD_SET(sockfd, &writefds);

    while (1) {
        retval = select(sockfd + 1, NULL, &writefds, NULL, &tv); //verifica se il socket è pronto all'invio dei dati
        //verifica se c'è errore con la restituzione di -1
        if (retval == -1) {
            perror("select()");

        }

        else if (retval) {
            //Pronto per l'invio dei dati
        recvfrom(sockfd, &pos, sizeof(pos), 0, (struct sockaddr*)&clientAddr, &clientAddrLen); //ricezione dati dal socket
            generateDebrisBasedOnSpaceship(pos.x);
            printf("ricevo in posizione %d\n",pos.x);

        } else {
            //Nessun dato disponibile, quindi genera detriti casuali
            initDebris();
        }

        //Caclola il tempo tracorso tra gli eventi, impostato a 2 secondi
        gettimeofday(&currentTime, NULL);
        long elapsedTime = (currentTime.tv_sec - lastDebrisTime.tv_sec) * 1000000L + (currentTime.tv_usec - lastDebrisTime.tv_usec);

        if (elapsedTime >= DEBRIS_GENERATION_INTERVAL) {
            lastDebrisTime = currentTime;
        }

        updateDebrisPositions();
        //Invio pacchetti al client
        sendDebrisPacket(sockfd, &clientAddr);
        // Controlla la velocità di aggiornamento dei detriti
        usleep(50000);
    }

    return 0;
}
