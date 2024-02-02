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
#define MAX_DETRITI 3
#define WINDOW_HEIGHT 10
#define DEBRIS_GENERATION_INTERVAL 2000000 // 2 secondi in microsecondi
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

void updateDebrisPositions() {
    for (int i = 0; i < GRID_SIZE; i++) {
        if (debris[i].active) {
            debris[i].y += 1;
            if (debris[i].y > WINDOW_HEIGHT) {
                debris[i].y = 0; // Riporta il detrito in cima per simulare un flusso continuo
                debris[i].x = rand() % GRID_SIZE; // Cambia la posizione x per varietà
            }
        }
    }
}

void sendDebrisPacket(int sockfd, struct sockaddr_in *clientAddr) {
    struct DebrisPacket packet;
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
    struct sockaddr_in servaddr;
    struct timeval lastDebrisTime, currentTime;
    fd_set writefds;
    struct timeval tv;
    int retval;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Binding fallito");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    memset(debris, 0, sizeof(debris));
    gettimeofday(&lastDebrisTime, NULL);
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    ssize_t bytesReceived;
    struct SpaceshipPosition pos;
    initDebris();
    // Azzera l'insieme di file descriptor e aggiunge il socket
    FD_ZERO(&writefds);
    FD_SET(sockfd, &writefds);

    while (1) {
        retval = select(sockfd + 1, NULL, &writefds, NULL, &tv);
        if (retval == -1) {
            perror("select()");
        } else if (retval) {
        recvfrom(sockfd, &pos, sizeof(pos), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
            generateDebrisBasedOnSpaceship(pos.x);
            printf("ricevo in posizione %d",pos.x);

        } else {
            // Nessun dato disponibile: genera detriti casuali
            initDebris();
        }
        gettimeofday(&currentTime, NULL);
        long elapsedTime = (currentTime.tv_sec - lastDebrisTime.tv_sec) * 1000000L + (currentTime.tv_usec - lastDebrisTime.tv_usec);

        if (elapsedTime >= DEBRIS_GENERATION_INTERVAL) {
            lastDebrisTime = currentTime;
        }

        updateDebrisPositions();
        sendDebrisPacket(sockfd, &clientAddr);
        usleep(50000); // Controlla la velocità di aggiornamento dei detriti
    }

    return 0;
}
