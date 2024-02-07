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
#define MAX_DEBRIS 50 // Numero massimo di detriti che possono essere gestiti contemporaneamente
#define MAX_DETRITI 4
struct Debris {
    int x, y;
    int active; // 1 se il detrito è attivo, 0 altrimenti
};

struct Spaceship {
    int x, y; // Posizione della navicella
    int width, height; // Dimensioni della navicella
};

struct Spaceship spaceship;

//invio dei detriti
struct DebrisPacket {
    struct Debris debris[MAX_DEBRIS];
};

struct DebrisPacket debrisReceivedPacket;

//posizione navicella
struct SpaceshipPosition {
    int x;
};

struct SpaceshipPosition pos;

//forma dei detriti
void drawCircle(SDL_Renderer *renderer, int centerX, int centerY, int radius) {
    for (int y = -radius; y <= radius; y++)
        for (int x = -radius; x <= radius; x++)
            if (x * x + y * y <= radius * radius)
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
}

int main() {

    int sockfd;
    struct sockaddr_in servaddr;
    SDL_Event event;
    int quit = 0;

    //Creazione Sokcet UDP, utilizzando indirizzi AF_INTET
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    //Configurazione indirizzo socket del server
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.146"); //ip
    servaddr.sin_port = htons(PORT);

    //implementazione libreria SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Starship", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //Creazione navicella
    spaceship.x = WINDOW_WIDTH / 2; // Posiziona la navicella al centro in orizzontale
    spaceship.y = WINDOW_HEIGHT - 50; // Posiziona la navicella verso il fondo della finestra
    spaceship.width = 50; // Larghezza navicella
    spaceship.height = 30; //Altezza navicella

    int verticalMargin = 500; // Margine verticale sopra l'astronave per attivare l'allarme
    while (!quit) {

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); //Sfondo nero
        SDL_RenderClear(renderer); // Pulizia frame corrente

        // Gestione eventi di input
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {

                    //Input tastiera
                    case SDLK_LEFT:
                        spaceship.x -= 100; //Sposta la navicella a sinistra
                        //Controllo per non far uscire la navicella dallo schermo
                        if (spaceship.x < 0) spaceship.x = 0;
                        break;

                    //Input tastiera
                    case SDLK_RIGHT:
                        spaceship.x += 100; //Sposta la navicella a destra
                        //Controllo per non far uscire la navicella dallo schermo
                        if (spaceship.x > WINDOW_WIDTH - spaceship.width) {
                            spaceship.x = WINDOW_WIDTH - spaceship.width;
                        }
                        break;
                    //Input Tastiera
                    case SDLK_ESCAPE:
                        quit = 1; // Premendo "Esc", esci dal gioco
                        break;
                }
            }
        }
        //Aggiorna la posizione della navicella da inviare al server
        pos.x = spaceship.x;
        printf("invio in posizione %d\n",pos.x);
        //Invia la posizione aggiornata al server
        sendto(sockfd, &pos, sizeof(pos), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

        // Ricezione dei detriti dal server
if(recvfrom(sockfd, &debrisReceivedPacket, sizeof(struct DebrisPacket), 0, NULL,NULL)<0)
{ perror("recvfrom failed");
            continue;
        }
        for (int i = 0; i < sizeof(struct DebrisPacket) ; i++) {
            printf("RICEVO in %d \n",debrisReceivedPacket.debris[i].x);
        }
                int alert=0; // Flag che parte da 0, se il detrito è in traiettoria diventa 1
        //Rendering dei detriti
        for (int i = 0; i < MAX_DETRITI; i++) {
            if (debrisReceivedPacket.debris[i].active) {
                int centerX = debrisReceivedPacket.debris[i].x * (WINDOW_WIDTH / GRID_SIZE) + (WINDOW_WIDTH / GRID_SIZE / 2);
                int centerY = debrisReceivedPacket.debris[i].y * (WINDOW_HEIGHT / GRID_SIZE) + (WINDOW_HEIGHT / GRID_SIZE / 2);

                // Verifica se il detrito è entro il margine verticale sopra l'astronave
                if (centerY >= spaceship.y - verticalMargin && centerY < spaceship.y + spaceship.height) {
                    if (centerX >= spaceship.x && centerX <= spaceship.x + spaceship.width) {
                        alert = 1; // Attiva l'allarme se il detrito è sopra l'astronave
                    }
                }

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //colore dei detriti bianco
                drawCircle(renderer, centerX, centerY, 20); //Chiamata per disegnare il detrito
            }
        }

        // Rendering dell'astronave
        SDL_Rect spaceshipRect = {spaceship.x, spaceship.y, spaceship.width, spaceship.height};
        if (alert) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0,255); // Imposta il colore dell'astronave su rosso in caso di allarme
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //Usa il colore bianco in assenza di detriti
        }
        SDL_RenderFillRect(renderer, &spaceshipRect); // Disegna l'astronave

        SDL_RenderPresent(renderer); // Aggiorna lo schermo con il nuovo frame renderizzato
    }

// Pulizia e chiusura
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    close(sockfd); //Chiusura socket

    return 0;
}