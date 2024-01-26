#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define PORT 12345
#define GRID_SIZE 10
#define DEBRIS_COUNT 5
#define WINDOW_HEIGHT 10 // Assumi una certa "altezza della finestra" per la simulazione

struct Debris {
    int x, y;
};

struct Debris debris[DEBRIS_COUNT]; // Array globale per i detriti

void initDebris() {
    for (int i = 0; i < DEBRIS_COUNT; i++) {
        debris[i].x = random() % GRID_SIZE;
        debris[i].y = -(random() % WINDOW_HEIGHT); // Inizio dall'alto
    }
}

void updateDebrisPositions() {
    for (int i = 0; i < DEBRIS_COUNT; i++) {
        debris[i].y += 1; // Fai "cadere" i detriti verso il basso

        if (debris[i].y > WINDOW_HEIGHT) {
            debris[i].y = 0; // Riporta il detrito in alto, mantenendo la posizione x invariata
        }
    }
}

void sendDebrisPositions(int sockfd, struct sockaddr_in *clientAddr) {
        if (sendto(sockfd, debris, sizeof(debris), 0, (struct sockaddr *)clientAddr, sizeof(*clientAddr)) < 0) {
            perror("sendto failed");
    }
}

int main() {
    int sockfd;
    struct sockaddr_in cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = inet_addr("172.20.10.3"); // Imposta l'indirizzo IP del client
    cliaddr.sin_port = htons(PORT);

    srand(time(0)); // Inizializzazione del generatore di numeri casuali

    initDebris(); // Inizializza i detriti


    while (1) {
        updateDebrisPositions(); // Aggiorna la posizione dei detriti
        printf("Sending debris positions...\n");
        sendDebrisPositions(sockfd, &cliaddr);
        usleep(50000);
    }

    close(sockfd);
    return 0;
}
