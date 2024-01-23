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

struct Debris {
    int x, y;
};
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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    //implementazione libreria SDL//

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Space Debris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Binding fallito");
        exit(EXIT_FAILURE);
    }

    while (1) {
        while (!quit) {
            // Gestione degli eventi
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    quit = 1;
                }
                // Altri eventi, come input da tastiera, possono essere gestiti qui
            }
            recvfrom(sockfd, debris, sizeof(debris), 0, NULL, NULL);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // Draw debris
            for (int i = 0; i < DEBRIS_COUNT; i++) {
                SDL_Rect rect = {debris[i].x * (WINDOW_WIDTH / GRID_SIZE), debris[i].y * (WINDOW_HEIGHT / GRID_SIZE),
                                 WINDOW_WIDTH / GRID_SIZE, WINDOW_HEIGHT / GRID_SIZE};
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &rect);
            }

            SDL_RenderPresent(renderer);
            SDL_Delay(100); // Delay for visual stability
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    close(sockfd);

    return 0;
}
