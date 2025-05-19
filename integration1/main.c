#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

// Function to create fade transition between two surfaces
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

// Function to start the game (launch prog from integration directory)
void startGame(SDL_Surface* screen, SDL_Surface* menu4) {
    // Create fade transition to menu4.png
    SDL_Surface* current = SDL_GetVideoSurface();
    fadeTransition(screen, current, menu4, 500);
    
    // Launch game executable
    pid_t pid = fork();
    if (pid == 0) {
        if(chdir("integration") != 0) {
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
    }
    else {
        printf("Failed to fork: %s\n", strerror(errno));
    }
}

int main(int argc, char** argv) {
    SDL_Surface *ecran, *image = NULL;
    SDL_Surface *btn_mono = NULL, *btn_multi = NULL, *btn_retour = NULL;
    SDL_Surface *btn_avatar1 = NULL, *btn_avatar2 = NULL, *btn_valider = NULL;
    SDL_Surface *gro_mono = NULL, *gro_multi = NULL, *gro_retour = NULL;
    SDL_Surface *gro_avatar1 = NULL, *gro_avatar2 = NULL, *gro_valider = NULL;
    SDL_Surface *menu4 = NULL;  // New surface for the final menu
    
    SDL_Rect positionimage, pos_mono, pos_multi, pos_retour;
    SDL_Rect pos_avatar1, pos_avatar2, pos_valider;
    
    int quitter = 1;
    int play = 0;
    int show_initial_buttons = 1;
    int show_new_buttons = 0;
    int bouton_hover = -1;
    int avatar_selectionne = 0;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    TTF_Init();

    ecran = SDL_SetVideoMode(1024, 1024, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    
    // Load all images
    image = IMG_Load("menu2.jpg");
    btn_mono = IMG_Load("mono.jpeg");
    btn_multi = IMG_Load("multi.jpeg");
    btn_retour = IMG_Load("retour.jpeg");
    btn_avatar1 = IMG_Load("avatar1.jpeg");
    btn_avatar2 = IMG_Load("avatar2.jpeg");
    btn_valider = IMG_Load("valider.jpeg");
    gro_mono = IMG_Load("gromono.jpeg");
    gro_multi = IMG_Load("gromulti.jpeg");
    gro_retour = IMG_Load("groretour.jpeg");
    gro_avatar1 = IMG_Load("groavatar1.jpeg");
    gro_avatar2 = IMG_Load("groavatar2.jpeg");
    gro_valider = IMG_Load("grovalider.jpeg");
    menu4 = IMG_Load("menu4.png");  // Load the final menu image
    
    // Set positions
    positionimage.x = 0; positionimage.y = 0;
    pos_mono.x = 100; pos_mono.y = 300;
    pos_multi.x = 600; pos_multi.y = 300;
    pos_retour.x = 650; pos_retour.y = 850;
    pos_avatar1.x = 100; pos_avatar1.y = 300;
    pos_avatar2.x = 600; pos_avatar2.y = 300;
    pos_valider.x = 350; pos_valider.y = 570;
    
    Mix_Music *musique = Mix_LoadMUS("palestine.mp3");
    Mix_Chunk *son_hover = Mix_LoadWAV("button_hover.wav");
    Mix_PlayMusic(musique, -1);

    while (quitter) {
        SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
        SDL_BlitSurface(image, NULL, ecran, &positionimage);

        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        SDL_Surface *btn_surface = NULL;
        SDL_Rect *btn_rect = NULL;

        if (show_initial_buttons == 1) {
            SDL_BlitSurface(btn_mono, NULL, ecran, &pos_mono);
            SDL_BlitSurface(btn_multi, NULL, ecran, &pos_multi);
            SDL_BlitSurface(btn_retour, NULL, ecran, &pos_retour);
        } 
        else if (show_new_buttons == 1) {
            SDL_BlitSurface(btn_avatar1, NULL, ecran, &pos_avatar1);
            SDL_BlitSurface(btn_avatar2, NULL, ecran, &pos_avatar2);
            SDL_BlitSurface(btn_valider, NULL, ecran, &pos_valider);
            SDL_BlitSurface(btn_retour, NULL, ecran, &pos_retour);
        }

        // Check hover effects
        if (show_initial_buttons == 1) {
            if (mouse_x >= pos_mono.x && mouse_x <= pos_mono.x + btn_mono->w &&
                mouse_y >= pos_mono.y && mouse_y <= pos_mono.y + btn_mono->h) {
                btn_surface = gro_mono;
                btn_rect = &pos_mono;
                if (bouton_hover != 1) {
                    Mix_PlayChannel(-1, son_hover, 0);
                    bouton_hover = 1;
                }
            } 
            else if (mouse_x >= pos_multi.x && mouse_x <= pos_multi.x + btn_multi->w &&
                     mouse_y >= pos_multi.y && mouse_y <= pos_multi.y + btn_multi->h) {
                btn_surface = gro_multi;
                btn_rect = &pos_multi;
                if (bouton_hover != 2) {
                    Mix_PlayChannel(-1, son_hover, 0);
                    bouton_hover = 2;
                }
            }
            else if (mouse_x >= pos_retour.x && mouse_x <= pos_retour.x + btn_retour->w &&
                     mouse_y >= pos_retour.y && mouse_y <= pos_retour.y + btn_retour->h) {
                btn_surface = gro_retour;
                btn_rect = &pos_retour;
                if (bouton_hover != 3) {
                    Mix_PlayChannel(-1, son_hover, 0);
                    bouton_hover = 3;
                }
            }
        } 
        else if (show_new_buttons == 1) {
            if (mouse_x >= pos_avatar1.x && mouse_x <= pos_avatar1.x + btn_avatar1->w &&
                mouse_y >= pos_avatar1.y && mouse_y <= pos_avatar1.y + btn_avatar1->h) {
                btn_surface = gro_avatar1;
                btn_rect = &pos_avatar1;
                if (bouton_hover != 4) {
                    Mix_PlayChannel(-1, son_hover, 0);
                    bouton_hover = 4;
                }
            } 
            else if (mouse_x >= pos_avatar2.x && mouse_x <= pos_avatar2.x + btn_avatar2->w &&
                     mouse_y >= pos_avatar2.y && mouse_y <= pos_avatar2.y + btn_avatar2->h) {
                btn_surface = gro_avatar2;
                btn_rect = &pos_avatar2;
                if (bouton_hover != 5) {
                    Mix_PlayChannel(-1, son_hover, 0);
                    bouton_hover = 5;
                }
            } 
            else if (mouse_x >= pos_valider.x && mouse_x <= pos_valider.x + btn_valider->w &&
                     mouse_y >= pos_valider.y && mouse_y <= pos_valider.y + btn_valider->h) {
                btn_surface = gro_valider;
                btn_rect = &pos_valider;
                if (bouton_hover != 6) {
                    Mix_PlayChannel(-1, son_hover, 0);
                    bouton_hover = 6;
                }
            }
            else if (mouse_x >= pos_retour.x && mouse_x <= pos_retour.x + btn_retour->w &&
                     mouse_y >= pos_retour.y && mouse_y <= pos_retour.y + btn_retour->h) {
                btn_surface = gro_retour;
                btn_rect = &pos_retour;
                if (bouton_hover != 7) {
                    Mix_PlayChannel(-1, son_hover, 0);
                    bouton_hover = 7;
                }
            }
        }

        if (btn_surface) {
            SDL_BlitSurface(btn_surface, NULL, ecran, btn_rect);
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quitter = 0;
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (show_initial_buttons == 1) {
                            if (event.button.x >= pos_mono.x && event.button.x <= pos_mono.x + btn_mono->w &&
                                event.button.y >= pos_mono.y && event.button.y <= pos_mono.y + btn_mono->h) {
                                show_initial_buttons = 0;
                                show_new_buttons = 1;
                            }

                            if (event.button.x >= pos_multi.x && event.button.x <= pos_multi.x + btn_multi->w &&
                                event.button.y >= pos_multi.y && event.button.y <= pos_multi.y + btn_multi->h) {
                                show_initial_buttons = 0;
                                show_new_buttons = 1;
                            }
                        }

                        if (show_new_buttons == 1) {
                            if (event.button.x >= pos_avatar1.x && event.button.x <= pos_avatar1.x + btn_avatar1->w &&
                                event.button.y >= pos_avatar1.y && event.button.y <= pos_avatar1.y + btn_avatar1->h) {
                                avatar_selectionne = 1; 
                            } 
                            else if (event.button.x >= pos_avatar2.x && event.button.x <= pos_avatar2.x + btn_avatar2->w &&
                                     event.button.y >= pos_avatar2.y && event.button.y <= pos_avatar2.y + btn_avatar2->h) {
                                avatar_selectionne = 2; 
                            }

                            if (event.button.x >= pos_valider.x && event.button.x <= pos_valider.x + btn_valider->w &&
                                event.button.y >= pos_valider.y && event.button.y <= pos_valider.y + btn_valider->h) {
                                if (avatar_selectionne != 0) {
                                    printf("Avatar %d sélectionné et validé\n", avatar_selectionne);
                                    startGame(ecran, menu4);
                                    quitter = 0;
                                }
                            }
                            
                            if (event.button.x >= pos_retour.x && event.button.x <= pos_retour.x + btn_retour->w &&
                                event.button.y >= pos_retour.y && event.button.y <= pos_retour.y + btn_retour->h) {
                                show_new_buttons = 0;
                                show_initial_buttons = 1;
                            }
                        }
                    }
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        quitter = 0;
                    }
                    break;
            }
        }

        SDL_Flip(ecran);
    }

    // Clean up
    SDL_FreeSurface(gro_mono);
    SDL_FreeSurface(gro_multi);
    SDL_FreeSurface(gro_retour);
    SDL_FreeSurface(gro_avatar1);
    SDL_FreeSurface(gro_avatar2);
    SDL_FreeSurface(gro_valider);
    SDL_FreeSurface(btn_valider);
    SDL_FreeSurface(btn_avatar1);
    SDL_FreeSurface(btn_avatar2);
    SDL_FreeSurface(btn_retour);
    SDL_FreeSurface(btn_mono);
    SDL_FreeSurface(btn_multi);
    SDL_FreeSurface(image);
    SDL_FreeSurface(menu4);
    
    Mix_FreeChunk(son_hover);
    Mix_FreeMusic(musique);
    Mix_CloseAudio();
    
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}
