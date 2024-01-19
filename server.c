#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 12345
#define M 7
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
    cliaddr.sin_addr.s_addr = inet_addr("192.168.1.122"); // Imposta l'indirizzo IP del client
    cliaddr.sin_port = htons(PORT);

    srand(time(0)); // Inizializzazione del generatore di numeri casuali

    while (1) {
        // Genera e invia pacchetti rappresentanti detriti nella griglia
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                if (rand() % 2 == 1) { // Genera casualmente un detrito (1) o vuoto (0)
                    char buffer[1024];
                    snprintf(buffer, sizeof(buffer), "Detrito in posizione (%d, %d)", i, j);
                    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
                }
            }
        }

        sleep(2); // Aspetta 2 secondi tra un'onda di meteoriti e l'altra
    }
    close(sockfd);
    return 0;
}
