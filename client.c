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
#define PORT 12345
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


void drawCircle(SDL_Renderer *renderer, int centerX, int centerY, int radius) {
    for (int y = -radius; y <= radius; y++)
        for (int x = -radius; x <= radius; x++)
            if (x * x + y * y <= radius * radius)
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
}

int main() {
    int alert=0;
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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    //implementazione libreria SDL//

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Starship", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Binding fallito");
        exit(EXIT_FAILURE);
    }
    struct Spaceship spaceship;
    spaceship.x = WINDOW_WIDTH / 2; // Posiziona la navicella al centro in orizzontale
    spaceship.y = WINDOW_HEIGHT - 50; // Posiziona la navicella verso il fondo della finestra
    spaceship.width = 50; // Imposta una larghezza
    spaceship.height = 30; // Imposta un'altezza

    while (!quit) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Nero per lo sfondo
        SDL_RenderClear(renderer); // Pulisci lo schermo per il nuovo frame

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        spaceship.x -= 30; // Sposta a sinistra
                        if (spaceship.x < 0) spaceship.x = 0; // Controlla i limiti dello schermo
                        break;
                    case SDLK_RIGHT:
                        spaceship.x += 30; // Sposta a destra
                        if (spaceship.x > WINDOW_WIDTH - spaceship.width) spaceship.x = WINDOW_WIDTH - spaceship.width; // Controlla i limiti
                        break;
                }
            }
        }

        if (recvfrom(sockfd, debris, sizeof(debris), 0, NULL, NULL) < 0) {
            perror("recvfrom failed");
            continue;
        }

        // Rendering dei detriti
        for (int i = 0; i < DEBRIS_COUNT; i++) {
            int centerX = debris[i].x * (WINDOW_WIDTH / GRID_SIZE) + (WINDOW_WIDTH / GRID_SIZE / 2);
            int centerY = debris[i].y * (WINDOW_HEIGHT / GRID_SIZE) + (WINDOW_HEIGHT / GRID_SIZE / 2);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Bianco per i detriti
            drawCircle(renderer, centerX, centerY, 30); // Usa la tua funzione drawCircle
        }

        // Rendering della navicella
        SDL_Rect spaceshipRect = {spaceship.x, spaceship.y, spaceship.width, spaceship.height};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Bianco per la navicella
        SDL_RenderFillRect(renderer, &spaceshipRect);

        // Controllo delle collisioni
        for (int i = 0; i < DEBRIS_COUNT; i++) {
            SDL_Rect debrisRect = {debris[i].x * (WINDOW_WIDTH / GRID_SIZE), debris[i].y * (WINDOW_HEIGHT / GRID_SIZE), 30, 30}; // Approssima i detriti come rettangoli per la collisione
            if (SDL_HasIntersection(&spaceshipRect, &debrisRect)) {
                printf("Collision detected!\n"); // Gestisci la collisione qui
                 quit = 1; // Decommenta per terminare il gioco in caso di collisione
            }
        }
        for(int i=0;i<DEBRIS_COUNT;i++){
            if (debris[i].active) {
                // Controlla se il detrito è sulla traiettoria della navicella
                if (debris[i].x >= spaceship.x && debris[i].x <= spaceship.x + spaceship.width) {
                    alert = 1; // Imposta l'allerta
                    break; // Esce dal ciclo se viene trovato un detrito sulla traiettoria
                }
            }
        }
        // Cambia il colore della navicella in caso di allerta
        if (alert) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rosso per l'allerta
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Bianco normalmente
        }
        SDL_RenderFillRect(renderer, &spaceshipRect);


        SDL_RenderPresent(renderer); // Aggiorna lo schermo con il rendering appena eseguito
        // SDL_Delay(100); // Decommenta per controllare il frame rate
    }

// Pulizia
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    close(sockfd);

    return 0;
}
