#include "header.h"

int main(int argc, char *argv[]) {
    AppState state = {0};
    state.currentVolume = 2;

    if (init(&state) != 0) {
        return 1;
    }

    while (1) {
        handleEvents(&state);
        render(&state);
    }

    cleanup(&state);
    return 0;
}
