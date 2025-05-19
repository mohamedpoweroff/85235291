#include "header.h"

// Initialisation des variables globales
int currentVolume = 2;
int i;
int isInOptionsMenu = 0;
int screenWidth = SCREEN_WIDTH ;  // Largeur initiale de la fenêtre
int screenHeight = SCREEN_HEIGHT; // Hauteur initiale de la fenêtre
SDL_Surface *screen, *background;
SDL_Surface *buttonIncrease, *buttonDecrease, *buttonFullscreen, *buttonNormal , *buttonReturn , *buttonDisplay , *buttonVolume;
SDL_Surface *buttonIncreaseHover, *buttonDecreaseHover, *buttonFullscreenHover, *buttonNormalHover , *buttonReturnHover , *buttonReturnHover;
SDL_Surface *currentButtonIncrease, *currentButtonDecrease, *currentButtonFullscreen, *currentButtonNormal, *currentButtonReturn , *currentButtonDisplay , *currentbuttonVolume; 
SDL_Surface *volumeBar[MAX_VOLUME + 1];
SDL_Rect buttonReturnRect , buttonDisplayRect , buttonVolumeRect;
Mix_Music *backgroundMusic = NULL;
Mix_Chunk *buttonClickSound = NULL;
Mix_Chunk *buttonHoverSound = NULL;

char *volumeBarFiles[MAX_VOLUME + 1] = {
    "images/barre0.png", "images/barre1.png", "images/barre2.png",
    "images/barre3.png", "images/barre4.png", "images/barre5.png"
};

// Initialisation du programme
int init(AppState *state) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    state->screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
    if (!state->screen) {
        printf("Erreur SDL_SetVideoMode: %s\n", SDL_GetError());
        return -1;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("Erreur IMG_Init: %s\n", IMG_GetError());
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Erreur Mix_OpenAudio: %s\n", IMG_GetError());
        return -1;
    }

    state->background = IMG_Load("images/haah.png");
    if (!state->background) {
        printf("Erreur chargement background: %s\n", IMG_GetError());
        return -1;
    }

    state->buttons[0] = IMG_Load("images/right.png");
    
    state->buttons[1] = IMG_Load("images/left.png");
    state->buttons[2] = IMG_Load("images/fullscreen.png");
    state->buttons[3] = IMG_Load("images/normal.png");
    state->buttons[4] = IMG_Load("images/return.png");
    state->buttons[5] = IMG_Load("images/display mode.png");
    state->buttons[6] = IMG_Load("images/volume.png");
    state->buttonsHover[0] = IMG_Load("images/rightH.png");
    state->buttonsHover[1] = IMG_Load("images/leftH.png");
    state->buttonsHover[2] = IMG_Load("images/fullscreenH.png");
    state->buttonsHover[3] = IMG_Load("images/normalH.png");
    state->buttonsHover[4] = IMG_Load("images/returnH.png");


    for (int i = 0; i < 5; i++) {
        if (!state->buttons[i] || !state->buttonsHover[i]) {
            printf("Erreur chargement boutons: %s\n", IMG_GetError());
            return -1;
        }
    }

    if (!state->buttons[5] || !state->buttons[6]) {
        printf("Erreur chargement boutons: %s\n", IMG_GetError());
        return -1;
    }

    for (int i = 0; i <= MAX_VOLUME; i++) {
        state->volumeBar[i] = IMG_Load(volumeBarFiles[i]);
        if (!state->volumeBar[i]) {
            printf("Erreur chargement barre de volume: %s\n", IMG_GetError());
            return -1;
        }
    }

    setButtonPositions(state);

    for (int i = 0; i < 7; i++) {
        state->currentButtons[i] = state->buttons[i];
    }

    state->backgroundMusic = Mix_LoadMUS("background.mp3");
    if (!state->backgroundMusic) {
        printf("Erreur chargement musique: %s\n", Mix_GetError());
        return -1;
    }

    state->buttonClickSound = Mix_LoadWAV("clic.wav");
    if (!state->buttonClickSound) {
        printf("Erreur chargement son de clic: %s\n", Mix_GetError());
        return -1;
    }

    state->buttonHoverSound = Mix_LoadWAV("hover.wav");
    if (!state->buttonHoverSound) {
        printf("Erreur chargement son de survol: %s\n", Mix_GetError());
        return -1;
    }

    Mix_PlayMusic(state->backgroundMusic, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME * state->currentVolume / MAX_VOLUME);

    return 0;
}

void setButtonPositions(AppState *state) {
    state->buttonRects[0] = (SDL_Rect){1000, 200, 160, 80};
    state->buttonRects[1] = (SDL_Rect){540, 200, 160, 80};
    state->buttonRects[2] = (SDL_Rect){540, 400, 160, 80};
    state->buttonRects[3] = (SDL_Rect){980, 400, 160, 80};
    state->buttonRects[4] = (SDL_Rect){950, 600, 160, 80};
    state->buttonRects[5] = (SDL_Rect){90, 400, 160, 80};
    state->buttonRects[6] = (SDL_Rect){90, 200, 160, 80};
}

void updateVolumeBar(AppState *state) {
    SDL_Rect rect = {700, 200, 300, 80};
    SDL_BlitSurface(state->volumeBar[state->currentVolume], NULL, state->screen, &rect);
}

void handleEvents(AppState *state) {
    SDL_Event event;
    int x, y;
    static int lastHoveredButton = -1;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            cleanup(state);
            exit(0);
        }

        SDL_GetMouseState(&x, &y);

        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                for (int i = 0; i < 7; i++) {
                    if (x >= state->buttonRects[i].x && x <= state->buttonRects[i].x + 160 &&
                        y >= state->buttonRects[i].y && y <= state->buttonRects[i].y + 80) {
                        if (i == 0 && state->currentVolume < MAX_VOLUME) {
                            state->currentVolume++;
                            Mix_VolumeMusic(MIX_MAX_VOLUME * state->currentVolume / MAX_VOLUME);
                        } else if (i == 1 && state->currentVolume > 0) {
                            state->currentVolume--;
                            Mix_VolumeMusic(MIX_MAX_VOLUME * state->currentVolume / MAX_VOLUME);
                        } else if (i == 2) {
                            toggleFullscreen(state, 1); // Switch to fullscreen
                        } else if (i == 3) {
                            toggleFullscreen(state, 0); // Switch to windowed mode
                        } else if (i == 4) {
                            state->isInOptionsMenu = 0;
                        }
                        Mix_PlayChannel(1, state->buttonClickSound, 0);
                    }
                }
                break;

            case SDL_MOUSEMOTION:
                for (int i = 0; i < 5; i++) {
                    if (x >= state->buttonRects[i].x && x <= state->buttonRects[i].x + 160 &&
                        y >= state->buttonRects[i].y && y <= state->buttonRects[i].y + 80) {
                        state->currentButtons[i] = state->buttonsHover[i];
                        if (lastHoveredButton != i) {
                            Mix_PlayChannel(-1, state->buttonHoverSound, 0);
                            lastHoveredButton = i;
                        }
                    } else {
                        state->currentButtons[i] = state->buttons[i];
                    }
                }
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_KP_PLUS|| (event.key.keysym.sym == SDLK_EQUALS&&event.key.keysym.mod==KMOD_LSHIFT)) {
                    if (state->currentVolume < MAX_VOLUME) {
                        state->currentVolume++;
                        Mix_VolumeMusic(MIX_MAX_VOLUME * state->currentVolume / MAX_VOLUME);
                        Mix_PlayChannel(1, state->buttonClickSound, 0);
                    }
                }

                if (event.key.keysym.sym == SDLK_MINUS || event.key.keysym.sym == SDLK_KP_MINUS) {
                    if (state->currentVolume > 0) {
                        state->currentVolume--;
                        Mix_VolumeMusic(MIX_MAX_VOLUME * state->currentVolume / MAX_VOLUME);
                        Mix_PlayChannel(1, state->buttonClickSound, 0);
                    }
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    state->isInOptionsMenu = 0;
                    Mix_PlayChannel(1, state->buttonClickSound, 0);
                }
                break;

            case SDL_VIDEORESIZE:
                if (state->isFullscreen) {
                    state->screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 32, SDL_SWSURFACE | SDL_FULLSCREEN);
                } else {
                    state->screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 32, SDL_SWSURFACE);
                }
                if (!state->screen) {
                    printf("Erreur SDL_SetVideoMode: %s\n", SDL_GetError());
                    return;
                }
                break;
        }
    }
}

void toggleFullscreen(AppState *state, int fullscreen) {
    Uint32 flags = SDL_SWSURFACE | SDL_DOUBLEBUF;
    if (fullscreen) {
        flags |= SDL_FULLSCREEN;
    }
    state->screen = SDL_SetVideoMode(0, 0, 32, flags);
    if (!state->screen) {
        printf("Erreur SDL_SetVideoMode: %s\n", SDL_GetError());
        return;
    }
    state->isFullscreen = fullscreen;

    // Clear the screen
    SDL_FillRect(state->screen, NULL, 0);

    // Draw the background
    SDL_BlitSurface(state->background, NULL, state->screen, NULL);

    // Draw the buttons
    for (int i = 0; i < 7; i++) {
        SDL_BlitSurface(state->currentButtons[i], NULL, state->screen, &state->buttonRects[i]);
    }

    // Update the volume bar
    updateVolumeBar(state);

    // Update the screen
    SDL_Flip(state->screen);
}

void cleanup(AppState *state) {
    SDL_FreeSurface(state->background);
    for (int i = 0; i < 7; i++) {
        SDL_FreeSurface(state->buttons[i]);
        SDL_FreeSurface(state->buttonsHover[i]);
    }

    Mix_FreeMusic(state->backgroundMusic);
    Mix_FreeChunk(state->buttonClickSound);
    Mix_FreeChunk(state->buttonHoverSound);
    for (int i = 0; i <= MAX_VOLUME; i++) {
        SDL_FreeSurface(state->volumeBar[i]);
    }
    IMG_Quit();
    SDL_Quit();
    Mix_CloseAudio();
}

void render(AppState *state) {
    // Clear the screen
    SDL_FillRect(state->screen, NULL, 0);

    // Draw the background
    SDL_BlitSurface(state->background, NULL, state->screen, NULL);

    // Draw the buttons
    for (int i = 0; i < 7; i++) {
        SDL_BlitSurface(state->currentButtons[i], NULL, state->screen, &state->buttonRects[i]);
    }

    // Update the volume bar
    updateVolumeBar(state);

    // Update the screen
    SDL_Flip(state->screen);
}
