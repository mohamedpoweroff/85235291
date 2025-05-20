#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

typedef struct {
    SDL_Surface* image;
    SDL_Rect position;
} Button;

SDL_Surface* resizeSurface(SDL_Surface* surface, float scale) {
    int newWidth = surface->w * scale;
    int newHeight = surface->h * scale;
    
    SDL_Surface* resized = SDL_CreateRGBSurface(
        surface->flags,
        newWidth,
        newHeight,
        surface->format->BitsPerPixel,
        surface->format->Rmask,
        surface->format->Gmask,
        surface->format->Bmask,
        surface->format->Amask
    );
    
    if (resized) {
        SDL_SoftStretch(surface, NULL, resized, NULL);
    }
    return resized;
}

Button createButton(const char* imagePath, float scale) {
    Button btn;
    SDL_Surface* original = IMG_Load(imagePath);
    
    if (!original) {
        printf("Failed to load image %s: %s\n", imagePath, IMG_GetError());
        exit(1);
    }
    
    btn.image = resizeSurface(original, scale);
    btn.position.x = 0;
    btn.position.y = 0;
    
    SDL_FreeSurface(original);
    return btn;
}

void fadeTransition(SDL_Surface* screen, SDL_Surface* from, SDL_Surface* to, int duration_ms) {
    const int steps = 30;
    const int delay = duration_ms / steps;
    
    for (int i = 0; i <= steps; i++) {
        float alpha = (float)i / steps;
        SDL_FillRect(screen, NULL, 0);
        SDL_BlitSurface(from, NULL, screen, NULL);
        SDL_SetAlpha(to, SDL_SRCALPHA, (Uint8)(alpha * 255));
        SDL_BlitSurface(to, NULL, screen, NULL);
        SDL_SetAlpha(to, SDL_SRCALPHA, 255);
        SDL_Flip(screen);
        SDL_Delay(delay);
    }
}

void startGame(SDL_Surface* screen) {
    Mix_HaltMusic();
    Mix_Music* gameMusic = Mix_LoadMUS("music2.mp3");
    if(!gameMusic) {
        printf("Failed to load game music: %s\n", Mix_GetError());
    } else {
        Mix_PlayMusic(gameMusic, -1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        if(chdir("integration1") != 0) {
            printf("Failed to change directory: %s\n", strerror(errno));
            exit(1);
        }
        execl("./prog", "./prog", NULL);
        printf("Failed to execute game: %s\n", strerror(errno));
        exit(1);
    } 
    else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if(gameMusic) {
            Mix_HaltMusic();
            Mix_FreeMusic(gameMusic);
        }
    }
    else {
        printf("Failed to fork: %s\n", strerror(errno));
    }
}

void startOptionProgram(SDL_Surface* screen) {
    Mix_HaltMusic();
    Mix_Music* gameMusic = Mix_LoadMUS("music2.mp3");
    if(!gameMusic) {
        printf("Failed to load game music: %s\n", Mix_GetError());
    } else {
        Mix_PlayMusic(gameMusic, -1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        if(chdir("kh") != 0) {
            printf("Failed to change directory: %s\n", strerror(errno));
            exit(1);
        }
        execl("./prog", "./prog", NULL);
        printf("Failed to execute option program: %s\n", strerror(errno));
        exit(1);
    } 
    else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if(gameMusic) {
            Mix_HaltMusic();
            Mix_FreeMusic(gameMusic);
        }
    }
    else {
        printf("Failed to fork: %s\n", strerror(errno));
    }
}

void startHistoryProgram(SDL_Surface* screen) {
    printf("History button clicked!\n");
    // Implement history program launch here
}

int main(int argc, char* argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    Mix_Music* music = Mix_LoadMUS("palestine.mp3");
    if(!music) {
        printf("Failed to load menu music: %s\n", Mix_GetError());
    } else {
        Mix_PlayMusic(music, -1);
    }

    SDL_Surface* menu1 = IMG_Load("back.jpeg");
    SDL_Surface* menu2 = IMG_Load("menu2.png");
    SDL_Surface* menu3 = IMG_Load("menu3.png");
    SDL_Surface* menu4 = IMG_Load("menu4.png");
    
    if(!menu1 || !menu2 || !menu3 || !menu4) {
        printf("Error loading backgrounds: %s\n", IMG_GetError());
        return 1;
    }

    // Create main menu buttons with scale 1.2
    Button jouerBtn = createButton("jouer.png", 1.2);
    Button optionBtn = createButton("option.png", 1.2);
    Button histoireBtn = createButton("histoire.png", 1.2);
    Button quitterBtn = createButton("quitter.png", 1.2);
    
    // Create page 2 buttons with same scale (1.2)
    Button nvBtn = createButton("nv.png", 1.2);
    Button enBtn = createButton("en.png", 1.2);

    // Position main menu buttons vertically at bottom center
    int buttonX = (1280 - jouerBtn.image->w) / 2;
    int startY = 720 - (jouerBtn.image->h * 4 + 30 * 3); // 30px space between buttons
    
    jouerBtn.position.x = buttonX;
    jouerBtn.position.y = startY;
    
    optionBtn.position.x = buttonX;
    optionBtn.position.y = startY + jouerBtn.image->h + 30;
    
    histoireBtn.position.x = buttonX;
    histoireBtn.position.y = startY + jouerBtn.image->h * 2 + 30 * 2;
    
    quitterBtn.position.x = buttonX;
    quitterBtn.position.y = startY + jouerBtn.image->h * 3 + 30 * 3;
    
    // Position page 2 buttons at bottom center with spacing
    int page2TotalWidth = nvBtn.image->w + enBtn.image->w + 40;
    int page2StartX = (1280 - page2TotalWidth) / 2;
    int page2ButtonY = 720 - nvBtn.image->h - 50; // 50px from bottom
    
    nvBtn.position.x = page2StartX;
    nvBtn.position.y = page2ButtonY;
    
    enBtn.position.x = page2StartX + nvBtn.image->w + 40;
    enBtn.position.y = page2ButtonY;

    SDL_Surface* screen = SDL_SetVideoMode(1280, 720, 32, SDL_SWSURFACE);

    #define LOADING_FRAMES 12
    SDL_Surface* loadingFrames[LOADING_FRAMES];
    int loadingIndex = 0;
    Uint32 lastFrameTime = SDL_GetTicks();

    char loadingPath[64];
    for (int i = 0; i < LOADING_FRAMES; i++) {
        sprintf(loadingPath, "loading/loading%d.png", i + 1);
        loadingFrames[i] = IMG_Load(loadingPath);
        if (!loadingFrames[i]) {
            printf("Error loading %s: %s\n", loadingPath, IMG_GetError());
            return 1;
        }
    }

    int quit = 0;
    int currentScreen = 1;
    Uint32 startLoadingTime = 0;

    while(!quit) {
        SDL_Event event;
        
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) quit = 1;
            
            if(event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                
                if(currentScreen == 1) {
                    if(mouseX >= jouerBtn.position.x && 
                       mouseX <= jouerBtn.position.x + jouerBtn.image->w &&
                       mouseY >= jouerBtn.position.y && 
                       mouseY <= jouerBtn.position.y + jouerBtn.image->h) {
                        currentScreen = 2;
                    }
                    else if(mouseX >= optionBtn.position.x && 
                            mouseX <= optionBtn.position.x + optionBtn.image->w &&
                            mouseY >= optionBtn.position.y && 
                            mouseY <= optionBtn.position.y + optionBtn.image->h) {
                        startOptionProgram(screen);
                    }
                    else if(mouseX >= histoireBtn.position.x && 
                            mouseX <= histoireBtn.position.x + histoireBtn.image->w &&
                            mouseY >= histoireBtn.position.y && 
                            mouseY <= histoireBtn.position.y + histoireBtn.image->h) {
                        startHistoryProgram(screen);
                    }
                    else if(mouseX >= quitterBtn.position.x && 
                            mouseX <= quitterBtn.position.x + quitterBtn.image->w &&
                            mouseY >= quitterBtn.position.y && 
                            mouseY <= quitterBtn.position.y + quitterBtn.image->h) {
                        quit = 1;
                    }
                } 
                else if(currentScreen == 2) {
                    if(mouseX >= nvBtn.position.x && 
                       mouseX <= nvBtn.position.x + nvBtn.image->w &&
                       mouseY >= nvBtn.position.y && 
                       mouseY <= nvBtn.position.y + nvBtn.image->h) {
                        fadeTransition(screen, menu2, menu3, 500);
                        currentScreen = 3;
                        startLoadingTime = SDL_GetTicks();
                    }
                    else if(mouseX >= enBtn.position.x && 
                            mouseX <= enBtn.position.x + enBtn.image->w &&
                            mouseY >= enBtn.position.y && 
                            mouseY <= enBtn.position.y + enBtn.image->h) {
                        printf("English button clicked!\n");
                    }
                }
            }
            
            if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
                else if(event.key.keysym.sym == SDLK_b) {
                    if(currentScreen == 3) {
                        fadeTransition(screen, menu3, menu1, 500);
                        currentScreen = 1;
                    }
                    else if(currentScreen == 2) {
                        fadeTransition(screen, menu2, menu1, 500);
                        currentScreen = 1;
                    }
                }
            }
        }

        if (quit) break;

        SDL_FillRect(screen, NULL, 0);

        SDL_Surface* bg = NULL;
        switch(currentScreen) {
            case 1: bg = menu1; break;
            case 2: bg = menu2; break;
            case 3: bg = menu3; break;
            case 4: bg = menu4; break;
        }
        SDL_BlitSurface(bg, NULL, screen, NULL);

        if(currentScreen == 1) {
            // Draw main menu buttons vertically at bottom
            SDL_BlitSurface(jouerBtn.image, NULL, screen, &jouerBtn.position);
            SDL_BlitSurface(optionBtn.image, NULL, screen, &optionBtn.position);
            SDL_BlitSurface(histoireBtn.image, NULL, screen, &histoireBtn.position);
            SDL_BlitSurface(quitterBtn.image, NULL, screen, &quitterBtn.position);
        }
        else if(currentScreen == 2) {
            // Draw page 2 buttons at bottom
            SDL_BlitSurface(nvBtn.image, NULL, screen, &nvBtn.position);
            SDL_BlitSurface(enBtn.image, NULL, screen, &enBtn.position);
        }
        else if(currentScreen == 3) {
            Uint32 now = SDL_GetTicks();
            if (now - lastFrameTime >= 100) {
                loadingIndex = (loadingIndex + 1) % LOADING_FRAMES;
                lastFrameTime = now;
            }

            SDL_Surface* currentFrame = loadingFrames[loadingIndex];
            SDL_Rect loadingPos;
            loadingPos.x = (1280 - currentFrame->w) / 2;
            loadingPos.y = (720 - currentFrame->h) / 2;
            SDL_BlitSurface(currentFrame, NULL, screen, &loadingPos);
            
            if (now - startLoadingTime >= 5000) {
                fadeTransition(screen, menu3, menu4, 500);
                currentScreen = 4;
                startGame(screen);
                quit = 1;
            }
        }

        SDL_Flip(screen);
        SDL_Delay(16);
    }

    // Cleanup
    SDL_FreeSurface(menu1);
    SDL_FreeSurface(menu2);
    SDL_FreeSurface(menu3);
    SDL_FreeSurface(menu4);
    SDL_FreeSurface(jouerBtn.image);
    SDL_FreeSurface(optionBtn.image);
    SDL_FreeSurface(histoireBtn.image);
    SDL_FreeSurface(quitterBtn.image);
    SDL_FreeSurface(nvBtn.image);
    SDL_FreeSurface(enBtn.image);
    for (int i = 0; i < LOADING_FRAMES; i++) {
        SDL_FreeSurface(loadingFrames[i]);
    }
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
