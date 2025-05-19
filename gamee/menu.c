// menu.c
// SDL 1.2 menu implementation for appearance/input selection
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BUTTON_COUNT 5
#define MAX_ENEMIES 10
#define MAX_NAME_LEN 16
#define MAX_SCORES 20

// Button identifiers
enum ButtonType { APPEARANCE1, APPEARANCE2, INPUT1, INPUT2, CONFIRM };

// Button structure
typedef struct {
    SDL_Rect rect;
    SDL_Surface* normal;
    SDL_Surface* highlighted;
    bool selected;
} Button;

// Button image filenames
const char* button_files[BUTTON_COUNT] = {
    "menu/buttons/appearance1.png",
    "menu/buttons/appearance2.png",
    "menu/buttons/input1.png",
    "menu/buttons/input2.png",
    "menu/buttons/confirm.png"
};
const char* button_files_h[BUTTON_COUNT] = {
    "menu/buttons/h/appearance1.png",
    "menu/buttons/h/appearance2.png",
    "menu/buttons/h/input1.png",
    "menu/buttons/h/input2.png",
    "menu/buttons/h/confirm.png"
};

// Button positions (x, y, w, h)
SDL_Rect button_rects[BUTTON_COUNT] = {
    {100, 200, 200, 60}, // appearance1
    {100, 300, 200, 60}, // appearance2
    {500, 200, 200, 60}, // input1
    {500, 300, 200, 60}, // input2
    {300, 400, 200, 60}  // confirm
};

// --- Game definitions ---
#define PLAYER_W 71
#define PLAYER_H 79
#define PLAYER_SPEED 5
#define JUMP_VELOCITY -15
#define GRAVITY 1
#define GROUND_Y 810
#define WALK_FRAMES 9
#define ATTACK_FRAMES 6
#define ATTACK_W 121
#define WALK_W 71

// Player structure
typedef struct {
    int x, y;
    int vx, vy;
    int on_ground;
    int facing_right;
    int attacking;
    int walk_frame;
    int attack_frame;
} Player;

// Enemy structure
typedef struct {
    int x, y, w, h;
    int alive;
    int hp;
} Enemy;

typedef struct {
    char name[MAX_NAME_LEN];
    int score;
} ScoreEntry;

void run_game();

// Function prototypes
void draw_menu(SDL_Surface* screen, SDL_Surface* bg, Button* buttons);
int handle_menu();
void draw_fade_and_text(SDL_Surface* screen, int alpha, const char* text, SDL_Color color, TTF_Font* font);
void spawn_enemy(Enemy* e, int hp, int camera_x);
void save_score(const char* name, int score);
int load_scores(ScoreEntry* entries, int max);
int cmp_score(const void* a, const void* b);
void show_score_menu(int final_score);
void show_best_scores();

void draw_menu(SDL_Surface* screen, SDL_Surface* bg, Button* buttons) {
    SDL_BlitSurface(bg, NULL, screen, NULL);
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        SDL_Surface* img = buttons[i].normal;
        if (buttons[i].selected) img = buttons[i].highlighted;
        SDL_BlitSurface(img, NULL, screen, &buttons[i].rect);
    }
    SDL_Flip(screen);
}

int handle_menu() {
    SDL_Surface* screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE);
    SDL_Surface* bg = IMG_Load("menu/background.png");
    Button buttons[BUTTON_COUNT];
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        buttons[i].rect = button_rects[i];
        buttons[i].normal = IMG_Load(button_files[i]);
        buttons[i].highlighted = IMG_Load(button_files_h[i]);
        buttons[i].selected = false;
    }
    int running = 1;
    int hover = -1;
    int selected_appearance = -1, selected_input = -1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_MOUSEMOTION) {
                hover = -1;
                for (int i = 0; i < BUTTON_COUNT; ++i) {
                    SDL_Rect r = buttons[i].rect;
                    int mx = e.motion.x, my = e.motion.y;
                    if (mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h) {
                        hover = i;
                    }
                }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                if (hover == APPEARANCE1 || hover == APPEARANCE2) {
                    selected_appearance = hover;
                }
                if (hover == INPUT1 || hover == INPUT2) {
                    selected_input = hover;
                }
                if (hover == CONFIRM && selected_appearance != -1 && selected_input != -1) {
                    // Save choices
                    FILE* f = fopen("menu/choices.txt", "w");
                    if (f) {
                        fprintf(f, "appearance=%d\ninput=%d\n", selected_appearance, selected_input - 2);
                        fclose(f);
                    }
                    running = 0;
                }
            }
        }
        for (int i = 0; i < BUTTON_COUNT; ++i) buttons[i].selected = false;
        if (hover != -1) buttons[hover].selected = true;
        if (selected_appearance != -1) buttons[selected_appearance].selected = true;
        if (selected_input != -1) buttons[selected_input].selected = true;
        draw_menu(screen, bg, buttons);
        SDL_Delay(16);
    }
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        SDL_FreeSurface(buttons[i].normal);
        SDL_FreeSurface(buttons[i].highlighted);
    }
    SDL_FreeSurface(bg);
    return 0;
}

void draw_fade_and_text(SDL_Surface* screen, int alpha, const char* text, SDL_Color color, TTF_Font* font) {
    SDL_Surface* fade = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(fade, NULL, SDL_MapRGBA(fade->format, 0, 0, 0, alpha));
    SDL_SetAlpha(fade, SDL_SRCALPHA, alpha);
    SDL_BlitSurface(fade, NULL, screen, NULL);
    SDL_FreeSurface(fade);
    if (text && font) {
        SDL_Surface* txt = TTF_RenderText_Solid(font, text, color);
        SDL_Rect dst = {SCREEN_WIDTH / 2 - txt->w / 2, SCREEN_HEIGHT / 2 - txt->h / 2, txt->w, txt->h};
        SDL_BlitSurface(txt, NULL, screen, &dst);
        SDL_FreeSurface(txt);
    }
}

void spawn_enemy(Enemy* e, int hp, int camera_x) {
    e->w = 60;
    e->h = 80;
    e->x = camera_x + 100 + rand() % (SCREEN_WIDTH - 200 - e->w);
    e->y = GROUND_Y - e->h;
    e->alive = 1;
    e->hp = hp;
}

void save_score(const char* name, int score) {
    FILE* f = fopen("score.txt", "a");
    if (f) {
        fprintf(f, "%s %d\n", name, score);
        fclose(f);
    }
}

int load_scores(ScoreEntry* entries, int max) {
    FILE* f = fopen("score.txt", "r");
    int n = 0;
    if (f) {
        while (n < max && fscanf(f, "%15s %d", entries[n].name, &entries[n].score) == 2) n++;
        fclose(f);
    }
    return n;
}

int cmp_score(const void* a, const void* b) {
    return ((ScoreEntry*)b)->score - ((ScoreEntry*)a)->score;
}

void show_score_menu(int final_score) {
    SDL_Surface* screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE);
    SDL_Surface* bg = IMG_Load("menu/background.png");
    TTF_Font* font = TTF_OpenFont("font.ttf", 48);
    char name[MAX_NAME_LEN] = "";
    int name_len = 0;
    int done = 0;
    SDL_Event e;
    while (!done) {
        SDL_BlitSurface(bg, NULL, screen, NULL);
        SDL_Color white = {255,255,255};
        SDL_Surface* prompt = TTF_RenderText_Solid(font, "Enter your name:", white);
        SDL_Rect pdst = {SCREEN_WIDTH/2 - prompt->w/2, 200, prompt->w, prompt->h};
        SDL_BlitSurface(prompt, NULL, screen, &pdst);
        SDL_FreeSurface(prompt);
        SDL_Surface* name_surf = TTF_RenderText_Solid(font, name, white);
        SDL_Rect ndst = {SCREEN_WIDTH/2 - 200, 300, 400, 60};
        SDL_FillRect(screen, &ndst, SDL_MapRGB(screen->format, 0,0,0));
        SDL_BlitSurface(name_surf, NULL, screen, &ndst);
        SDL_FreeSurface(name_surf);
        char score_str[32];
        sprintf(score_str, "Score: %d", final_score);
        SDL_Surface* s_surf = TTF_RenderText_Solid(font, score_str, white);
        SDL_Rect sdst = {SCREEN_WIDTH/2 - s_surf->w/2, 400, s_surf->w, s_surf->h};
        SDL_BlitSurface(s_surf, NULL, screen, &sdst);
        SDL_FreeSurface(s_surf);
        SDL_Flip(screen);
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit(0);
            if (e.type == SDL_KEYDOWN) {                if (e.key.keysym.sym == SDLK_RETURN && name_len > 0) {
                    save_score(name, final_score);
                    done = 1;
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && name_len > 0) {
                    name[--name_len] = '\0';
                } else if (name_len < MAX_NAME_LEN-1) {
                    Uint16 unicode = e.key.keysym.unicode;
                    if (unicode != 0) {
                        char c = (char)unicode;
                        if (isalnum(c) || c == '_' || c == '-' || c == ' ') {
                            name[name_len++] = c;
                            name[name_len] = '\0';
                        }
                    }
                }
            }
        }
    }
    SDL_FreeSurface(bg);
    TTF_CloseFont(font);
}

void show_best_scores() {
    SDL_Surface* screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE);
    SDL_Surface* board = IMG_Load("menu/board.png");
    SDL_Surface* title = IMG_Load("menu/best_score.png");
    TTF_Font* font = TTF_OpenFont("font.ttf", 48);
    ScoreEntry entries[MAX_SCORES];
    int n = load_scores(entries, MAX_SCORES);
    qsort(entries, n, sizeof(ScoreEntry), cmp_score);
    SDL_BlitSurface(board, NULL, screen, NULL);
    SDL_BlitSurface(title, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2-title->w/2, 40, title->w, title->h});
    SDL_Color white = {255,255,255};
    for (int i = 0; i < n && i < 10; ++i) {
        char line[64];
        sprintf(line, "%d. %s - %d", i+1, entries[i].name, entries[i].score);
        SDL_Surface* lsurf = TTF_RenderText_Solid(font, line, white);
        SDL_Rect ldst = {SCREEN_WIDTH/2 - lsurf->w/2, 200 + i*60, lsurf->w, lsurf->h};
        SDL_BlitSurface(lsurf, NULL, screen, &ldst);
        SDL_FreeSurface(lsurf);
    }
    SDL_Flip(screen);
    SDL_Event e;
    int wait = 1;
    while (wait) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)) wait = 0;
        }
    }
    SDL_FreeSurface(board);
    SDL_FreeSurface(title);
    TTF_CloseFont(font);
}

void run_game() {
    SDL_Surface* screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE);
    SDL_Surface* bg = IMG_Load("jeu/background.png");
    SDL_Surface* collisionmap = IMG_Load("jeu/collisionmap.png");
    SDL_Surface* walk_sheet = IMG_Load("jeu/joueur/walk.png");
    SDL_Surface* attack_sheet = IMG_Load("jeu/joueur/attack.png");
    TTF_Init();
    TTF_Font* font = TTF_OpenFont("font.ttf", 64);
    if (!bg || !collisionmap || !walk_sheet || !attack_sheet || !font) {
        fprintf(stderr, "Error loading game assets\n");
        return;
    }
    srand(time(NULL));
    Player player = {100, GROUND_Y - PLAYER_H, 0, 0, 1, 1, 0, 0, 0};
    int attack_anim_counter = 0;
    int running = 1;
    int frame = 0;
    int camera_x = 0;
    int camera_y = 0;
    int level = 1;
    int timer = 15;
    int fade = 0;
    int fade_in = 0;
    int fade_done = 0;
    int score = 0;
    Uint32 last_time = SDL_GetTicks();
    Enemy enemies[MAX_ENEMIES] = {0};
    int enemy_count = 3;
    int max_enemies = 3;
    for (int i = 0; i < enemy_count; ++i) spawn_enemy(&enemies[i], 1, camera_x);
    SDL_Event e;
    while (running) {
        Uint32 now = SDL_GetTicks();
        if (now - last_time >= 1000 && fade == 0) {
            timer--;
            last_time = now;
        }
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_LEFT) {
                    player.vx = -PLAYER_SPEED;
                    player.facing_right = 0;
                }
                if (e.key.keysym.sym == SDLK_RIGHT) {
                    player.vx = PLAYER_SPEED;
                    player.facing_right = 1;
                }
                if (e.key.keysym.sym == SDLK_UP && player.on_ground) {
                    player.vy = JUMP_VELOCITY;
                    player.on_ground = 0;
                }
                if (e.key.keysym.sym == SDLK_k && !player.attacking) {
                    player.attacking = 1;
                    player.attack_frame = 0;
                }
            }
            if (e.type == SDL_KEYUP) {
                if (e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_RIGHT) {
                    player.vx = 0;
                }
            }
        }
        // Physics
        player.x += player.vx;
        player.y += player.vy;
        if (!player.on_ground) player.vy += GRAVITY;
        if (player.y + PLAYER_H >= GROUND_Y) {
            player.y = GROUND_Y - PLAYER_H;
            player.vy = 0;
            player.on_ground = 1;
        }
        // Camera
        camera_x = player.x + (player.attacking ? ATTACK_W / 2 : WALK_W / 2) - SCREEN_WIDTH / 2;
        if (camera_x < 0) camera_x = 0;
        int bg_max_x = bg->w - SCREEN_WIDTH;
        if (camera_x > bg_max_x) camera_x = bg_max_x;
        camera_y = GROUND_Y + PLAYER_H - SCREEN_HEIGHT;
        if (camera_y < 0) camera_y = 0;
        // Animation
        if (player.attacking) {
            attack_anim_counter++;
            if (attack_anim_counter >= 6) {
                player.attack_frame++;
                attack_anim_counter = 0;
            }
            if (player.attack_frame >= ATTACK_FRAMES) player.attacking = 0;
        } else if (player.vx != 0 && player.on_ground) {
            if (frame % 6 == 0) player.walk_frame = (player.walk_frame + 1) % WALK_FRAMES;
        } else {
            player.walk_frame = 0;
        }
        // Attack collision
        if (player.attacking && player.attack_frame == 2) {
            for (int i = 0; i < enemy_count; ++i) {
                if (!enemies[i].alive) continue;
                int px = player.x + (player.facing_right ? WALK_W : -40);
                SDL_Rect atk = {px, player.y, player.facing_right ? ATTACK_W : 40, PLAYER_H};
                SDL_Rect er = {enemies[i].x, enemies[i].y, enemies[i].w, enemies[i].h};
                if (atk.x < er.x + er.w && atk.x + atk.w > er.x && atk.y < er.y + er.h && atk.y + atk.h > er.y) {
                    enemies[i].hp--;
                    if (enemies[i].hp <= 0) {
                        enemies[i].alive = 0;
                        score++;
                    }
                }
            }
        }
        // Draw
        SDL_Rect bg_src = {camera_x, camera_y, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_BlitSurface(bg, &bg_src, screen, NULL);
        // Draw enemies
        for (int i = 0; i < enemy_count; ++i) {
            if (!enemies[i].alive) continue;
            SDL_Rect er = {enemies[i].x - camera_x, enemies[i].y - camera_y, enemies[i].w, enemies[i].h};
            SDL_FillRect(screen, &er, SDL_MapRGB(screen->format, level == 1 ? 255 : 0, 0, 0));
        }
        // Draw player
        SDL_Rect src, dst;
        dst.x = (player.x - camera_x);
        dst.y = player.y - camera_y;
        SDL_Surface* current_sheet = player.attacking ? attack_sheet : walk_sheet;
        int current_frame = player.attacking ? player.attack_frame : player.walk_frame;
        if (player.attacking) {
            dst.w = ATTACK_W;
            dst.h = PLAYER_H;
            src.x = current_frame * ATTACK_W;
            src.y = 0;
            src.w = ATTACK_W;
            src.h = PLAYER_H;
        } else {
            dst.w = WALK_W;
            dst.h = PLAYER_H;
            src.x = current_frame * WALK_W;
            src.y = 0;
            src.w = WALK_W;
            src.h = PLAYER_H;
        }
        if (player.facing_right) {
            SDL_BlitSurface(current_sheet, &src, screen, &dst);
        } else {
            SDL_Surface* flipped = SDL_CreateRGBSurface(SDL_SWSURFACE, src.w, src.h, 32,
                current_sheet->format->Rmask, current_sheet->format->Gmask, current_sheet->format->Bmask, current_sheet->format->Amask);
            SDL_LockSurface(current_sheet);
            SDL_LockSurface(flipped);
            for (int y = 0; y < src.h; ++y) {
                for (int x = 0; x < src.w; ++x) {
                    Uint32 pixel = ((Uint32*)(((Uint8*)current_sheet->pixels) + (src.y + y) * current_sheet->pitch))[src.x + x];
                    ((Uint32*)((Uint8*)flipped->pixels + y * flipped->pitch))[src.w - 1 - x] = pixel;
                }
            }
            SDL_UnlockSurface(current_sheet);
            SDL_UnlockSurface(flipped);
            SDL_BlitSurface(flipped, NULL, screen, &dst);
            SDL_FreeSurface(flipped);
        }
        // Draw timer
        char tstr[16];
        sprintf(tstr, "%02d", timer);
        SDL_Color white = {255, 255, 255};
        SDL_Surface* ttxt = TTF_RenderText_Solid(font, tstr, white);
        SDL_Rect tdst = {20, 20, ttxt->w, ttxt->h};
        SDL_BlitSurface(ttxt, NULL, screen, &tdst);
        SDL_FreeSurface(ttxt);
        // Draw score
        char score_str[32];
        sprintf(score_str, "Score: %d", score);
        SDL_Surface* stxt = TTF_RenderText_Solid(font, score_str, white);
        SDL_Rect sdst = {20, 80, stxt->w, stxt->h};
        SDL_BlitSurface(stxt, NULL, screen, &sdst);        SDL_FreeSurface(stxt);
        // Fade and level transition from level 1 to level 2
        if (level == 1 && timer <= 0 && fade < 255 && !fade_done) fade += 5;
        if (level == 1 && fade >= 255 && !fade_done) {
            fade_done = 1;
            SDL_Delay(1000);
            fade_in = 1;
            timer = 30;
            level = 2;
            for (int i = 0; i < MAX_ENEMIES; ++i) enemies[i].alive = 0;
            spawn_enemy(&enemies[0], 2, camera_x);
            enemy_count = 1;
        }
        if (fade_in) {
            draw_fade_and_text(screen, fade, "LEVEL 2", (SDL_Color){255, 0, 0}, font);
            if (fade > 0) fade -= 5;
            else {
                fade_in = 0;
                fade = 0;
                fade_done = 0;
            }
        } else if (fade > 0) {
            draw_fade_and_text(screen, fade, "LEVEL 2", (SDL_Color){255, 0, 0}, font);
        }
        // Level 1 logic: respawn red rectangles up to max_enemies
        if (level == 1 && timer > 0) {
            int alive = 0;
            for (int i = 0; i < enemy_count; ++i) {
                if (enemies[i].alive) alive++;
            }
            for (int i = 0; i < enemy_count && alive < max_enemies; ++i) {
                if (!enemies[i].alive) {
                    spawn_enemy(&enemies[i], 1, camera_x);
                    alive++;
                }
            }
        }
        // Level 2 logic
        if (level == 2 && timer > 0) {
            int alive = 0;
            for (int i = 0; i < enemy_count; ++i) {
                if (enemies[i].alive) alive++;
            }
            for (int i = 0; i < enemy_count && alive < max_enemies; ++i) {
                if (!enemies[i].alive) {
                    spawn_enemy(&enemies[i], 2, camera_x);
                    alive++;
                }
            }
        }        // End level 2 when timer runs out
        if (level == 2 && timer <= 0) {
            if (!fade_done) {
                if (fade < 255) {
                    fade += 5;
                } else {
                    fade_done = 1;
                    SDL_Delay(1000);
                    show_score_menu(score);
                    show_best_scores();
                    running = 0;
                    continue;
                }
            }
        }
        SDL_Flip(screen);
        SDL_Delay(16);
        frame++;
    }
    SDL_FreeSurface(bg);
    SDL_FreeSurface(collisionmap);
    SDL_FreeSurface(walk_sheet);
    SDL_FreeSurface(attack_sheet);
    TTF_CloseFont(font);
    TTF_Quit();
}

int main(int argc, char* argv[]) {    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL init error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_EnableUNICODE(1);  // Enable Unicode text input
    atexit(SDL_Quit);
    handle_menu();
    run_game();
    return 0;
}
