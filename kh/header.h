#ifndef HEADER_H
#define HEADER_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>

// Définir la taille maximale du volume
#define MAX_VOLUME 5

// Screen dimensions
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Button dimensions
#define BUTTON_WIDTH 160
#define BUTTON_HEIGHT 80

typedef struct {
    int currentVolume;
    int isFullscreen;
    int isInOptionsMenu;
    SDL_Surface *screen, *background;
    SDL_Surface *buttons[7];
    SDL_Surface *buttonsHover[7];
    SDL_Surface *currentButtons[7];
    SDL_Surface *volumeBar[MAX_VOLUME + 1];
    SDL_Rect buttonRects[7];
    Mix_Music *backgroundMusic;
    Mix_Chunk *buttonClickSound;
    Mix_Chunk *buttonHoverSound;
} AppState;

// Déclarations des fonctions
int init(AppState *state);
void updateVolumeBar(AppState *state);
void handleEvents(AppState *state);
void cleanup(AppState *state);
void toggleFullscreen(AppState *state, int fullscreen);
void setButtonPositions(AppState *state);
void render(AppState *state);

#endif
