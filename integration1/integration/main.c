#define __u_char_defined
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define IDLE_FRAMES 4
#define MOVE_FRAMES 4
#define DEATH_FRAMES 4
#define ENEMY_MAX_HEALTH 6
#define HURT_FRAMES 1
#define MAX_OBSTACLES 3

// Structure pour sauvegarder l'état du jeu
typedef struct {
    SDL_Rect posPlayer;
    SDL_Rect posEnemy[2];
    int enemyHealth[2];
    bool isDying[2];
    bool obstacleActive[MAX_OBSTACLES];
    SDL_Rect barrierPos;
    int barrierDirection;
} GameState;

SDL_Surface* resizeImage(SDL_Surface* surface, int newWidth, int newHeight) {
    SDL_Surface* resized = SDL_CreateRGBSurface(SDL_SWSURFACE, newWidth, newHeight,
        surface->format->BitsPerPixel,
        surface->format->Rmask,
        surface->format->Gmask,
        surface->format->Bmask,
        surface->format->Amask);
    SDL_SoftStretch(surface, NULL, resized, NULL);
    return resized;
}

SDL_Surface* flipSurface(SDL_Surface *surface) {
    SDL_Surface *flipped = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h,
        surface->format->BitsPerPixel,
        surface->format->Rmask,
        surface->format->Gmask,
        surface->format->Bmask,
        surface->format->Amask);

    SDL_LockSurface(surface);
    SDL_LockSurface(flipped);

    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint32 pixel = ((Uint32*)surface->pixels)[y * surface->w + x];
            ((Uint32*)flipped->pixels)[y * surface->w + (surface->w - 1 - x)] = pixel;
        }
    }

    SDL_UnlockSurface(surface);
    SDL_UnlockSurface(flipped);

    return flipped;
}

bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

void saveGameState(const GameState *state) {
    FILE *file = fopen("saved_game.dat", "wb");
    if (file) {
        fwrite(state, sizeof(GameState), 1, file);
        fclose(file);
    }
}

bool loadGameState(GameState *state) {
    FILE *file = fopen("saved_game.dat", "rb");
    if (file) {
        fread(state, sizeof(GameState), 1, file);
        fclose(file);
        return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    SDL_Surface *background = IMG_Load("background.jpg");
    SDL_Surface *screen = SDL_SetVideoMode(background->w, background->h, 32, SDL_HWSURFACE);
    SDL_WM_SetCaption("el kaboul ddrmech", NULL);

    // Load minimap image
    SDL_Surface *minimapImage = IMG_Load("mini.jpeg");
    if (!minimapImage) {
        printf("Failed to load minimap image!\n");
        return 1;
    }
    SDL_Surface *minimap = resizeImage(minimapImage, 356, 156);
    SDL_FreeSurface(minimapImage);
    
    // Create dot surfaces for minimap
    SDL_Surface *redDot = SDL_CreateRGBSurface(SDL_SWSURFACE, 7, 12, 32, 0, 0, 0, 0);
    SDL_FillRect(redDot, NULL, SDL_MapRGB(redDot->format, 255, 0, 0));
    
    SDL_Surface *blueDot = SDL_CreateRGBSurface(SDL_SWSURFACE, 7, 12, 32, 0, 0, 0, 0);
    SDL_FillRect(blueDot, NULL, SDL_MapRGB(blueDot->format, 0, 0, 255));

    // Load obstacle images
    SDL_Surface *obstacles[MAX_OBSTACLES];
    SDL_Rect obstaclePos[MAX_OBSTACLES] = {
        {200, 780, 50, 30},
        {100, 780, 50, 30}, 
        {600, 780, 50, 30}
    };
    bool obstacleActive[MAX_OBSTACLES] = {true, true, true};
    
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "g%d.jpeg", i);
        obstacles[i] = IMG_Load(filename);
        if (!obstacles[i]) {
            printf("Failed to load obstacle image %s\n", filename);
            return 1;
        }
    }

    // Load vertical barrier image
    SDL_Surface *barrier = IMG_Load("barre.jpeg");
    if (!barrier) {
        printf("Failed to load barrier image!\n");
        return 1;
    }
    SDL_Rect barrierPos = {600, 0, barrier->h, barrier->w};
    bool barrierActive = true;
    int barrierDirection = 1;
    int barrierSpeed = 3;

    SDL_Surface *idleRight[IDLE_FRAMES], *idleLeft[IDLE_FRAMES];
    SDL_Surface *moveRight[MOVE_FRAMES], *moveLeft[MOVE_FRAMES];
    SDL_Surface *death[DEATH_FRAMES];
    SDL_Surface *healthBar[ENEMY_MAX_HEALTH];
    SDL_Surface *hurtLeft[HURT_FRAMES], *hurtRight[HURT_FRAMES];
    SDL_Surface *attackRight[MOVE_FRAMES], *attackLeft[MOVE_FRAMES];
    char filename[64];

    for (int i = 0; i < IDLE_FRAMES; ++i) {
        snprintf(filename, sizeof(filename), "idle/idle%d.png", i + 1);
        SDL_Surface *temp = IMG_Load(filename);
        SDL_Surface *resized = resizeImage(temp, temp->w / 4, temp->h / 4);
        idleLeft[i] = resized;
        idleRight[i] = flipSurface(resized);
        SDL_FreeSurface(temp);
    }

    for (int i = 0; i < MOVE_FRAMES; ++i) {
        snprintf(filename, sizeof(filename), "move/move%d.png", i + 1);
        SDL_Surface *temp = IMG_Load(filename);
        SDL_Surface *resized = resizeImage(temp, temp->w / 4, temp->h / 4);
        moveLeft[i] = resized;
        moveRight[i] = flipSurface(resized);
        SDL_FreeSurface(temp);
    }

    for (int i = 0; i < DEATH_FRAMES; ++i) {
        snprintf(filename, sizeof(filename), "death/death%d.png", i + 1);
        SDL_Surface *temp = IMG_Load(filename);
        death[i] = resizeImage(temp, temp->w / 4, temp->h / 4);
        SDL_FreeSurface(temp);
    }

    for (int i = 0; i < ENEMY_MAX_HEALTH; ++i) {
        snprintf(filename, sizeof(filename), "health/health%d.png", i + 1);
        SDL_Surface *temp = IMG_Load(filename);
        healthBar[i] = resizeImage(temp, temp->w / 5, temp->h / 5);
        SDL_FreeSurface(temp);
    }

    snprintf(filename, sizeof(filename), "hurt/hurt1.png");
    SDL_Surface *temp = IMG_Load(filename);
    hurtLeft[0] = resizeImage(temp, temp->w / 4, temp->h / 4);
    hurtRight[0] = flipSurface(hurtLeft[0]);
    SDL_FreeSurface(temp);

    for (int i = 0; i < MOVE_FRAMES; ++i) {
        snprintf(filename, sizeof(filename), "attack/attack%d.png", i + 1);
        SDL_Surface *temp = IMG_Load(filename);
        SDL_Surface *resized = resizeImage(temp, temp->w / 4, temp->h / 4);
        attackLeft[i] = resized;
        attackRight[i] = flipSurface(resized);
        SDL_FreeSurface(temp);
    }

    SDL_Surface *player = IMG_Load("me/me.png");
    SDL_Surface *resizedPlayer = resizeImage(player, player->w / 4, player->h / 4);
    SDL_FreeSurface(player);
    
    const int PLAYER_BASE_Y = 820;
    SDL_Rect posPlayer = {
        screen->w / 2 - resizedPlayer->w / 2,
        PLAYER_BASE_Y - resizedPlayer->h
    };

    SDL_Rect posEnemy[2] = {
        {100, 820 - idleRight[0]->h},
        {300, 820 - idleRight[0]->h}
    };
    
    int moveDirection[2] = {1, -1};
    int directionAnimationFrame[2] = {0};
    bool isChangingDirection[2] = {false};
    int enemyHealth[2] = {ENEMY_MAX_HEALTH, ENEMY_MAX_HEALTH};
    bool isDying[2] = {false};
    int deathFrame[2] = {0};
    bool isHurt[2] = {false};
    Uint32 hurtEndTime[2] = {0};
    int moveDistance = 2;
    int currentFrame = 0, frameDelay = 0;

    SDL_Event event;
    bool running = true;
    bool isAttacking = false;
    srand(time(NULL));

    SDL_Rect minimapPos = {10, 10};
    float scaleX = (float)minimap->w / background->w;
    float scaleY = (float)minimap->h / background->h;
    Uint32 blackColor = SDL_MapRGB(screen->format, 0, 0, 0);

    // Charger l'état sauvegardé si disponible
    GameState savedState;
    if (loadGameState(&savedState)) {
        posPlayer = savedState.posPlayer;
        for (int i = 0; i < 2; i++) {
            posEnemy[i] = savedState.posEnemy[i];
            enemyHealth[i] = savedState.enemyHealth[i];
            isDying[i] = savedState.isDying[i];
        }
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            obstacleActive[i] = savedState.obstacleActive[i];
        }
        barrierPos = savedState.barrierPos;
        barrierDirection = savedState.barrierDirection;
    }

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false; // Quitte immédiatement sans menu
            }
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s) {
                // Touche S pour sauvegarder sans quitter
                GameState currentState;
                currentState.posPlayer = posPlayer;
                for (int i = 0; i < 2; i++) {
                    currentState.posEnemy[i] = posEnemy[i];
                    currentState.enemyHealth[i] = enemyHealth[i];
                    currentState.isDying[i] = isDying[i];
                }
                for (int i = 0; i < MAX_OBSTACLES; i++) {
                    currentState.obstacleActive[i] = obstacleActive[i];
                }
                currentState.barrierPos = barrierPos;
                currentState.barrierDirection = barrierDirection;
                
                saveGameState(&currentState);
            }
        }

        // [Reste du code de la boucle de jeu inchangé...]
        
        Uint8 *keystates = SDL_GetKeyState(NULL);
        if (keystates[SDLK_LEFT]) posPlayer.x -= 4;
        if (keystates[SDLK_RIGHT]) posPlayer.x += 4;

        barrierPos.y += barrierSpeed * barrierDirection;
        if (barrierPos.y <= 0) {
            barrierPos.y = 0;
            barrierDirection = 1;
        } else if (barrierPos.y + barrierPos.h >= background->h) {
            barrierPos.y = background->h - barrierPos.h;
            barrierDirection = -1;
        }

        if (checkCollision(posPlayer, barrierPos)) {
            if (posPlayer.x + posPlayer.w/2 < barrierPos.x + barrierPos.w/2) {
                posPlayer.x = barrierPos.x - posPlayer.w;
            } else {
                posPlayer.x = barrierPos.x + barrierPos.w;
            }
        }

        if (keystates[SDLK_e] && !isAttacking) {
            isAttacking = true;
            for (int i = 0; i < 2; i++) {
                SDL_Rect enemyRect = {posEnemy[i].x, posEnemy[i].y, idleRight[0]->w, idleRight[0]->h};
                SDL_Rect playerRect = {posPlayer.x, posPlayer.y, resizedPlayer->w, resizedPlayer->h};
                if (checkCollision(enemyRect, playerRect) && enemyHealth[i] > 0 && !isDying[i]) {
                    enemyHealth[i]--;
                    if (enemyHealth[i] < 0) enemyHealth[i] = 0;
                    isHurt[i] = true;
                    hurtEndTime[i] = SDL_GetTicks() + 200;
                }
            }
        }

        if (!keystates[SDLK_e]) {
            isAttacking = false;
        }

        frameDelay++;
        if (frameDelay >= 5) {
            currentFrame = (currentFrame + 1) % IDLE_FRAMES;
            frameDelay = 0;
        }

        SDL_BlitSurface(background, NULL, screen, NULL);

        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (obstacleActive[i]) {
                SDL_BlitSurface(obstacles[i], NULL, screen, &obstaclePos[i]);
                if (checkCollision(posPlayer, obstaclePos[i])) {
                    obstacleActive[i] = false;
                }
            }
        }

        SDL_BlitSurface(barrier, NULL, screen, &barrierPos);

        for (int i = 0; i < 2; i++) {
            if (!isChangingDirection[i] && !isDying[i] && (rand() % 100 < 1)) {
                isChangingDirection[i] = true;
                directionAnimationFrame[i] = 0;
                moveDirection[i] *= -1;
            }

            if (!isChangingDirection[i] && !isDying[i]) {
                posEnemy[i].x += moveDirection[i] * moveDistance;
                if (checkCollision(posEnemy[i], barrierPos)) {
                    moveDirection[i] *= -1;
                    posEnemy[i].x += moveDirection[i] * moveDistance * 2;
                }
                if (posEnemy[i].x < 0 || posEnemy[i].x > background->w - idleRight[0]->w)
                    moveDirection[i] *= -1;
            }

            if (isDying[i]) {
                if (deathFrame[i] < DEATH_FRAMES * 6) {
                    SDL_BlitSurface(death[deathFrame[i] / 6], NULL, screen, &posEnemy[i]);
                    deathFrame[i]++;
                }
            } else if (isHurt[i]) {
                SDL_BlitSurface((moveDirection[i] == 1 ? hurtRight[0] : hurtLeft[0]), NULL, screen, &posEnemy[i]);
                if (SDL_GetTicks() > hurtEndTime[i]) isHurt[i] = false;
            } else if (isChangingDirection[i]) {
                SDL_BlitSurface((moveDirection[i] == 1 ? moveRight : moveLeft)[directionAnimationFrame[i]], NULL, screen, &posEnemy[i]);
                directionAnimationFrame[i]++;
                if (directionAnimationFrame[i] >= MOVE_FRAMES) isChangingDirection[i] = false;
            } else {
                SDL_BlitSurface((moveDirection[i] == 1 ? idleRight : idleLeft)[currentFrame], NULL, screen, &posEnemy[i]);
            }

            if (enemyHealth[i] <= 0 && !isDying[i]) {
                isDying[i] = true;
            }

            if (!isDying[i] && enemyHealth[i] > 0) {
                SDL_Rect healthPos = {screen->w - healthBar[0]->w - 50, 20 + i * 40};
                SDL_BlitSurface(healthBar[ENEMY_MAX_HEALTH - enemyHealth[i]], NULL, screen, &healthPos);
            }
        }

        SDL_BlitSurface(resizedPlayer, NULL, screen, &posPlayer);
        SDL_BlitSurface(minimap, NULL, screen, &minimapPos);
        
        SDL_Rect playerDotPos = {
            minimapPos.x + (int)((posPlayer.x + resizedPlayer->w/2) * scaleX) - blueDot->w/2,
            minimapPos.y + (int)((posPlayer.y + resizedPlayer->h/2) * scaleY) - blueDot->h/2
        };
        SDL_BlitSurface(blueDot, NULL, screen, &playerDotPos);
        
        for (int i = 0; i < 2; i++) {
            if (!isDying[i]) {
                SDL_Rect enemyDotPos = {
                    minimapPos.x + (int)((posEnemy[i].x + idleRight[0]->w/2) * scaleX) - redDot->w/2,
                    minimapPos.y + (int)((posEnemy[i].y + idleRight[0]->h/2) * scaleY) - redDot->h/2
                };
                SDL_BlitSurface(redDot, NULL, screen, &enemyDotPos);
            }
        }
        
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (obstacleActive[i]) {
                SDL_Rect obstacleDotPos = {
                    minimapPos.x + (int)((obstaclePos[i].x + obstaclePos[i].w/2) * scaleX) - 2,
                    minimapPos.y + (int)((obstaclePos[i].y + obstaclePos[i].h/2) * scaleY) - 2,
                    8, 8
                };
                SDL_FillRect(screen, &obstacleDotPos, blackColor);
            }
        }
        
        SDL_Rect barrierDotPos = {
            minimapPos.x + (int)((barrierPos.x + barrierPos.w/2) * scaleX) - 2,
            minimapPos.y + (int)((barrierPos.y + barrierPos.h/2) * scaleY) - 2,
            4, (int)(barrierPos.h * scaleY)
        };
        SDL_FillRect(screen, &barrierDotPos, SDL_MapRGB(screen->format, 128, 128, 128));

        SDL_Flip(screen);
        SDL_Delay(16);
    }

    // Nettoyage
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        SDL_FreeSurface(obstacles[i]);
    }
    SDL_FreeSurface(background);
    SDL_FreeSurface(minimap);
    SDL_FreeSurface(redDot);
    SDL_FreeSurface(blueDot);
    SDL_FreeSurface(barrier);
    for (int i = 0; i < IDLE_FRAMES; ++i) SDL_FreeSurface(idleLeft[i]);
    for (int i = 0; i < MOVE_FRAMES; ++i) SDL_FreeSurface(moveLeft[i]);
    for (int i = 0; i < DEATH_FRAMES; ++i) SDL_FreeSurface(death[i]);
    for (int i = 0; i < ENEMY_MAX_HEALTH; ++i) SDL_FreeSurface(healthBar[i]);
    SDL_FreeSurface(resizedPlayer);

    SDL_Quit();
    IMG_Quit();
    return 0;
}
