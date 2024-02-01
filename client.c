#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <SDL2/SDL.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PORT 4550
#define GRID_SIZE 10
#define DEBRIS_COUNT 5
#define MAX_DEBRIS 50 // Numero massimo di detriti che possono essere gestiti contemporaneamente


struct Debris {
    int x, y;
    int active; // 1 se il detrito è attivo, 0 altrimenti
};
struct Spaceship {
    int x, y; // Posizione della navicella
    int width, height; // Dimensioni della navicella
};

struct Debris debris[MAX_DEBRIS]; // Array per gestire più generazioni di detriti
struct DebrisPacket {
    struct Debris debris[MAX_DEBRIS];
};

struct DebrisPacket debrisReceivedPacket;


void drawCircle(SDL_Renderer *renderer, int centerX, int centerY, int radius) {
    for (int y = -radius; y <= radius; y++)
        for (int x = -radius; x <= radius; x++)
            if (x * x + y * y <= radius * radius)
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    struct Debris debris[DEBRIS_COUNT];
    SDL_Event event;
    int quit = 0;


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.115"); //ip
    servaddr.sin_port = htons(PORT);
    //implementazione libreria SDL//

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Starship", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                          WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    // Messaggio di prova da inviare
    char *message = "Questo è un messaggio di prova";

    // Invia il messaggio al server
    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("Errore nell'invio del messaggio");
        close(sockfd);
        exit(1);
    }
    printf("Messaggio inviato con successo al server.\n");

    struct Spaceship spaceship;
    spaceship.x = WINDOW_WIDTH / 2; // Posiziona la navicella al centro in orizzontale
    spaceship.y = WINDOW_HEIGHT - 50; // Posiziona la navicella verso il fondo della finestra
    spaceship.width = 50; // Imposta una larghezza
    spaceship.height = 30; // Imposta un'altezza

    int verticalMargin = 500; // Margine verticale sopra l'astronave per attivare l'allarme
    while (!quit) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Imposta il colore di sfondo su nero
        SDL_RenderClear(renderer); // Pulisci il frame corrente
        // Gestione degli eventi di input
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        spaceship.x -= 50; // Sposta a sinistra
                        //if (spaceship.x < 0) spaceship.x = 0; // Evita di uscire dallo schermo
                        break;
                    case SDLK_RIGHT:
                        spaceship.x += 50; // Sposta a destra
                        if (spaceship.x > WINDOW_WIDTH - spaceship.width){
                            spaceship.x = WINDOW_WIDTH -spaceship.width; // Evita di uscire dallo schermo
                    case SDLK_ESCAPE:
                        quit = 1; // Premendo "Esc", esci dal gioco
                        break;}
                }
            }
        }

        // Ricezione dei detriti dal server
        int receivedDebrisCount=recvfrom(sockfd, &debrisReceivedPacket, sizeof(struct DebrisPacket), 0, NULL,NULL);
                if (receivedDebrisCount < 0) {
            perror("recvfrom failed");
            continue;
        }
        int alert=0; // Resetta l'allarme per ogni frame
        // Rendering dei detriti
        for (int i = 0; i < receivedDebrisCount; i++) {
            if (debrisReceivedPacket.debris[i].active) {
                int centerX = debrisReceivedPacket.debris[i].x * (WINDOW_WIDTH / GRID_SIZE) + (WINDOW_WIDTH / GRID_SIZE / 2);
                int centerY = debrisReceivedPacket.debris[i].y * (WINDOW_HEIGHT / GRID_SIZE) + (WINDOW_HEIGHT / GRID_SIZE / 2);

                // Verifica se il detrito è entro il margine verticale sopra l'astronave
                if (centerY >= spaceship.y - verticalMargin && centerY < spaceship.y + spaceship.height) {
                    if (centerX >= spaceship.x && centerX <= spaceship.x + spaceship.width) {
                        alert = 1; // Attiva l'allarme se il detrito è direttamente sopra l'astronave entro il margine verticale
                    }
                }

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Imposta il colore dei detriti su bianco
                drawCircle(renderer, centerX, centerY, 20); // Disegna il detrito come un cerchio
            }
        }

        // Rendering dell'astronave
        SDL_Rect spaceshipRect = {spaceship.x, spaceship.y, spaceship.width, spaceship.height};
        int score=0;
        for (int i = 0; i < DEBRIS_COUNT; i++) {
            SDL_Rect debrisRect = {debris[i].x * (WINDOW_WIDTH / GRID_SIZE), debris[i].y * (WINDOW_HEIGHT / GRID_SIZE), 30, 30}; // Approssima i detriti come rettangoli per la collisione
            if (SDL_HasIntersection(&spaceshipRect, &debrisRect)) {
                printf("Collision detected!\n"); // Gestisci la collisione qui
                score=score+1; // Decommenta per terminare il gioco in caso di collisione
            }else{
                score=score-10;
            }
            //printf("\nScore:\n %d",score);
        }
        if (alert) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0,
                                   255); // Imposta il colore dell'astronave su rosso in caso di allarme
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Altrimenti, usa il colore bianco
        }
        SDL_RenderFillRect(renderer, &spaceshipRect); // Disegna l'astronave

        SDL_RenderPresent(renderer); // Aggiorna lo schermo con il nuovo frame renderizzato
    }

// Pulizia e chiusura
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    close(sockfd);

    return 0;
}