#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#define PORT 12345
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
    memset(&packet, 0, sizeof(struct DebrisPacket));

    for (int i = 0; i < GRID_SIZE; i++) {
        if (debris[i].active) {
            packet.debris[i] = debris[i];
        }
    }

    sendto(sockfd, &packet, sizeof(struct DebrisPacket), 0, (struct sockaddr *)clientAddr, sizeof(*clientAddr));
}

int main() {
    int sockfd;
    struct sockaddr_in cliaddr;
    struct timeval lastDebrisTime, currentTime;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = inet_addr("10.22.53.22"); //ip
    cliaddr.sin_port = htons(PORT);

    srand(time(NULL));
    memset(debris, 0, sizeof(debris));
    gettimeofday(&lastDebrisTime, NULL);
    initDebris();

    while (1) {
        gettimeofday(&currentTime, NULL);
        long elapsedTime = (currentTime.tv_sec - lastDebrisTime.tv_sec) * 1000000L + (currentTime.tv_usec - lastDebrisTime.tv_usec);

        if (elapsedTime >= DEBRIS_GENERATION_INTERVAL) {
            lastDebrisTime = currentTime;
        }

        updateDebrisPositions();
        sendDebrisPacket(sockfd, &cliaddr);
        usleep(50000); // Controlla la velocità di aggiornamento dei detriti
    }

    close(sockfd);
    return 0;
}
