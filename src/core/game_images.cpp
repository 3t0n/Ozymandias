#include "SDL.h"

#include "core/file.h"
#include "core/io.h"
#include "core/log.h"
#include "core/mods.h"
#include "core/config.h"
#include "core/game_environment.h"
#include "core/table_translation.h"
#include "core/image.h"
#include "core/game_images.h"

enum {
    NO_EXTRA_FONT = 0,
    FULL_CHARSET_IN_FONT = 1,
    MULTIBYTE_IN_FONT = 2
};

game_images::game_images():
    current_climate(-1), editor(false), fonts_enabled(false),
    font_base_offset(0), terrain_ph_offset(0), enemy_id(0),
    font_encoding(ENCODING_WESTERN_EUROPE) {
}

int game_images::image_groupid_translation(int table[], int group) {
    if (group > 99999) {
        group -= 99999;
    } else {
        for (int i = 0; table[i] < GROUP_MAX_GROUP; i += 2) {
            if (table[i] == group) {
                return table[i + 1];
            }
        }
    }

    // missing entry!!!!
    return group;
}
int32_t game_images::get_image_id(int group) {
    switch (GAME_ENV) {
        case ENGINE_ENV_C3:
            return get_collection(MAIN_FILENAME_C3).get_id(group);
        case ENGINE_ENV_PHARAOH:
            group = image_groupid_translation(groupid_translation_table_ph, group);

            // All collections loaded one by one without free spaces,
            // so, every image group have shift for number of all previous groups in collections
            // For example: group >= 0 && group < 66 for the terrain collection and
            // group >= 66 && group < 294 for the main collection
            size_t right_bound = 0;
            for (auto& collection: collections) {
                size_t collection_size = collection.get_num_group_records();
                right_bound += collection_size - 1;
                int32_t left_bound = right_bound - collection_size - 1;

                if (group > left_bound && group < right_bound) {
                    int32_t shifted_group = group - right_bound + collection_size - 1;
                    return collection.get_id(shifted_group);
                }
            }
    }
    return -1;
}

int image_id_from_group(int group) {
    return game_images::get().get_image_id(group);
}

image *game_images::get_image(int id) {
    image* img = &image::dummy();
    for (auto& collection: collections) {
        img = collection.get_image(id);
        if (!img->is_dummy()) {
            return img;
        }
    }

    SDL_Log("Image with group id '%d' not found", id);
    return img;
}

image *game_images::get_image(const char* search_tag) {
    image* img = &image::dummy();
    for (auto& collection: collections) {
        img = collection.get_image(search_tag);
        if (!img->is_dummy()) {
            return img;
        }
    }

    SDL_Log("Image with tag '%s' not found", search_tag);
    return img;
}

image *image_get(int id) {
    return game_images::get().get_image(id);
}

const image *game_images::image_letter(int letter_id) {
    if (fonts_enabled == FULL_CHARSET_IN_FONT) {
        return get_font().get_image(font_base_offset + letter_id);
    } else if (fonts_enabled == MULTIBYTE_IN_FONT && letter_id >= IMAGE_FONT_MULTIBYTE_OFFSET) {
        return get_font().get_image(font_base_offset + letter_id - IMAGE_FONT_MULTIBYTE_OFFSET);
    } else if (letter_id < IMAGE_FONT_MULTIBYTE_OFFSET) {
        return get_image(get_image_id(GROUP_FONT) + letter_id);
    }

    return &image::dummy();
}

const image *image_letter(int letter_id) {
    return game_images::get().image_letter(letter_id);
}

const image *game_images::image_get_enemy(int id) {
    return get_enemy().get_image(id);
}

const image *image_get_enemy(int id) {
    return game_images::get().image_get_enemy(id);
}

const color_t *game_images::image_data(int id) {
    const image *lookup = get_image(id);
    image *img = get_image(id + lookup->get_offset_mirror());
    return image_data(img);
}

const color_t *game_images::image_data(image* img) {
    return img->get_data();
}

const color_t *image_data(int id) {
    return game_images::get().image_data(id);
}

const color_t *image_data(image* img) {
    return game_images::get().image_data(img);
}

const color_t *game_images::image_data_letter(int letter_id) {
    return image_letter(letter_id)->get_data();
}

const color_t *image_data_letter(int letter_id) {
    return game_images::get().image_data_letter(letter_id);
}

const color_t *game_images::image_data_enemy(int id) {
    const image *lookup = get_image(id);
    const image *img = get_image(id + lookup->get_offset_mirror());
    id += img->get_offset_mirror();
    if (img->get_offset() > 0) {
        return img->get_data();
    }
    return nullptr;
}

const color_t *image_data_enemy(int id) {
    return game_images::get().image_data_enemy(id);
}

bool game_images::load_main(int climate_id, int is_editor, int force_reload) {
    if (climate_id != current_climate || force_reload) {
        for (auto &collection : collections) {
            collection.load_files();
        }
//        print();

        current_climate = climate_id;
        is_editor = is_editor;
    }
    return true;
}

bool game_images::load_enemy(int new_enemy_id) {
    bool result = false;
    if (enemy_id == new_enemy_id) {
        result = true;
    } else {
        result = get_enemy().load_files();
        enemy_id = new_enemy_id;
    }
    return result;
}

bool game_images::load_fonts(encoding_type new_encoding) {
    bool result = false;
    if (font_encoding == new_encoding) {
        result = true;
    } else {

        if (new_encoding == ENCODING_CYRILLIC) {
            result = false;
        } else if (new_encoding == ENCODING_TRADITIONAL_CHINESE) {
            result = false;
        } else if (new_encoding == ENCODING_SIMPLIFIED_CHINESE) {
            result = false;
        } else if (new_encoding == ENCODING_KOREAN) {
            result = false;
        } else {
            fonts_enabled = NO_EXTRA_FONT;
        }

        // TODO: support different fonts
        result = get_font().load_files();
        font_encoding = new_encoding;
    }
    return result;
}

int game_images::get_current_climate() const {
    return current_climate;
}

void game_images::set_current_climate(int new_climate) {
    current_climate = new_climate;
}

bool game_images::is_editor() const {
    return editor;
}

void game_images::set_editor(bool new_editor) {
    editor = new_editor;
}

bool game_images::is_fonts_enabled() const {
    return fonts_enabled;
}

void game_images::set_fonts_enabled(bool new_fonts_enabled) {
    fonts_enabled = new_fonts_enabled;
}

int game_images::get_font_base_offset() const {
    return font_base_offset;
}

void game_images::set_font_base_offset(int new_font_base_offset) {
    font_base_offset = new_font_base_offset;
}

int game_images::get_terrain_ph_offset() const {
    return terrain_ph_offset;
}

void game_images::set_terrain_ph_offset(int new_terrain_ph_offset) {
    terrain_ph_offset = new_terrain_ph_offset;
}

const image_collection& game_images::get_collection(std::string& collection_name) {
    for (auto& c: collections) {
        if (c.get_filename() == collection_name) {
            return c;
        }
    }
    return image_collection::dummy();
}

const image_collection &game_images::get_collection(const char *collection_name) {
    std::string string_name(collection_name);
    return get_collection(string_name);
}

image_collection &game_images::get_enemy() {
    std::string enemy_filename;
    if (GAME_ENV == ENGINE_ENV_C3) {
        enemy_filename = ENEMY_FILENAMES_C3[enemy_id];
    } else if (GAME_ENV == ENGINE_ENV_PHARAOH) {
        enemy_filename = ENEMY_FILENAMES_PH[enemy_id];
    }

    return const_cast<image_collection &>(get_collection(enemy_filename));
}

image_collection &game_images::get_font() {
    std::string font_filename;
    if (GAME_ENV == ENGINE_ENV_C3) {
        font_filename = FONTS_FILENAMES_C3[0];
    } else if (GAME_ENV == ENGINE_ENV_PHARAOH) {
        font_filename = FONTS_FILENAMES_PH[0];
    }

    return const_cast<image_collection &>(get_collection(font_filename));
}

size_t game_images::get_images_shift(std::string &collection_name) {
    size_t result = 0;
    for (auto& c: collections) {
        if (c.get_filename() != collection_name) {
            result += c.get_num_image_records();
        } else {
            break;
        }
    }
    return result;
}

size_t game_images::get_groups_shift(std::string &collection_name) {
    size_t result = 0;
    for (auto& c: collections) {
        if (c.get_filename() != collection_name) {
            result += c.get_num_group_records() - 1;
        } else {
            break;
        }
    }
    return result;
}

void game_images::print() {
    for (auto& c: collections) {
        std::string collection_name(c.get_filename());
        SDL_Log("---");
        SDL_Log("Collection '%s': shift for groups: %zu, shift for images: %zu",
                collection_name.c_str(), get_groups_shift(collection_name), get_images_shift(collection_name));
        c.print();
    }
}
