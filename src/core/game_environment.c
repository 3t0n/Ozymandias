#include "game_environment.h"

#include "SDL.h"

#include <assert.h>

static int game_engine = ENGINE_ENV_MAX;
static const char *game_name;
static const char *pref_filename;
static bool debug_mode = false;

void init_debug_mode(bool debug_flag) {
    debug_mode = debug_flag;
}

void init_game_environment(int env) {
    assert(env != ENGINE_ENV_MAX);
    game_engine = env;

    if (is_game_caesar3()) {
        game_name = "Caesar 3";
        pref_filename = "data_dir_c3.txt";
    } else if (is_game_pharaoh()) {
        game_name = "Pharaoh";
        pref_filename = "data_dir_pharaoh.txt";
    }

    SDL_Log("Engine set to %s", game_name);
}
void assert_env_init(void) {
    assert(game_engine < ENGINE_ENV_MAX);
}

const char *get_game_title(void) {
    assert_env_init();
    return game_name;
}
const char *get_engine_pref_path(void) {
    assert_env_init();
    return pref_filename;
}

engine_sizes env_sizes(void) {
    assert_env_init();
    engine_sizes result = {};

    if (is_game_caesar3()) {
        result = {
                20,
                20,
                20,
                20,

                4,
                8,
                8,

                50,

                32,
                65,
                64,
                522,

                250
        };
    }

    if (is_game_pharaoh()) {
        result = {
                20,
                20,
                20,
                20,

                4,
                8,
                8,

                114,

                32,
                65,
                64,
                522,

                50
        };
    }

    return result;
}

bool is_debug_mode() {
    return debug_mode;
}

bool is_game_pharaoh() {
    return game_engine == ENGINE_ENV_PHARAOH;
}

bool is_game_caesar3() {
    return game_engine == ENGINE_ENV_C3;
}

int get_game_engine() {
    return game_engine;
}

