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

typedef struct font_files_collection {
    const char CYRILLIC_FONTS_555[NAME_SIZE];
    const char CYRILLIC_FONTS_SG2[NAME_SIZE];
    const char TRAD_CHINESE_FONTS_555[NAME_SIZE];
    const char KOREAN_FONTS_555[NAME_SIZE];
} font_files_collection;

font_files_collection ffcs[] = {
        {
                "C3_fonts.sg2",
                "C3_fonts.555",
                "rome.555",
                "korean.555",
        },
        {
                "",
                "",
                "",
                ""
        }
};

struct graphics_files_collection {
    const char C3_MAIN_555[3][NAME_SIZE];
    const char C3_MAIN_SG2[3][NAME_SIZE];
    const char C3_EDITOR_555[3][NAME_SIZE];
    const char C3_EDITOR_SG2[3][NAME_SIZE];
    const char C3_EMPIRE_555[NAME_SIZE];
    const char C3_ENEMY_555[20][NAME_SIZE];
    const char C3_ENEMY_SG2[20][NAME_SIZE];
    const char PH_MAIN_555[NAME_SIZE];
    const char PH_MAIN_SG3[NAME_SIZE];
    const char PH_UNLOADED_555[NAME_SIZE];
    const char PH_UNLOADED_SG3[NAME_SIZE];
    const char PH_TERRAIN_555[NAME_SIZE];
    const char PH_TERRAIN_SG3[NAME_SIZE];
    const char PH_FONTS_555[NAME_SIZE];
    const char PH_FONTS_SG3[NAME_SIZE];
    const char PH_EDITOR_GRAPHICS_555[NAME_SIZE];
    const char PH_EDITOR_GRAPHICS_SG3[NAME_SIZE];
    const char PH_EMPIRE_555[NAME_SIZE];
    const char PH_EMPIRE_SG3[NAME_SIZE];
    const char PH_ENEMY_555[20][NAME_SIZE];
    const char PH_ENEMY_SG2[20][NAME_SIZE];
    const char PH_EXPANSION_555[NAME_SIZE];
    const char PH_EXPANSION_SG3[NAME_SIZE];
    const char PH_SPRMAIN_555[NAME_SIZE];
    const char PH_SPRMAIN_SG3[NAME_SIZE];
    const char PH_SPRMAIN2_555[NAME_SIZE];
    const char PH_SPRMAIN2_SG3[NAME_SIZE];
    const char PH_SPRAMBIENT_555[NAME_SIZE];
    const char PH_SPRAMBIENT_SG3[NAME_SIZE];

} gfc = {
        {
                "c3.555",
                "c3_north.555",
                "c3_south.555"
        },
        {
                "c3.sg2",
                "c3_north.sg2",
                "c3_south.sg2"
        },
        {
                "c3map.555",
                "c3map_north.555",
                "c3map_south.555"
        },
        {
                "c3map.sg2",
                "c3map_north.sg2",
                "c3map_south.sg2"
        },
        "The_empire.555",
        {
                "goths.555",
                "Etruscan.555",
                "Etruscan.555",
                "carthage.555",
                "Greek.555",
                "Greek.555",
                "egyptians.555",
                "Persians.555",
                "Phoenician.555",
                "celts.555",
                "celts.555",
                "celts.555",
                "Gaul.555",
                "Gaul.555",
                "goths.555",
                "goths.555",
                "goths.555",
                "Phoenician.555",
                "North African.555",
                "Phoenician.555",
        },
        {
                "goths.sg2",
                "Etruscan.sg2",
                "Etruscan.sg2",
                "carthage.sg2",
                "Greek.sg2",
                "Greek.sg2",
                "egyptians.sg2",
                "Persians.sg2",
                "Phoenician.sg2",
                "celts.sg2",
                "celts.sg2",
                "celts.sg2",
                "Gaul.sg2",
                "Gaul.sg2",
                "goths.sg2",
                "goths.sg2",
                "goths.sg2",
                "Phoenician.sg2",
                "North African.sg2",
                "Phoenician.sg2",
        },
        "data/Pharaoh_General.555",
        "data/Pharaoh_General.sg3",
        "data/Pharaoh_Unloaded.555",
        "data/Pharaoh_Unloaded.sg3",
        "data/Pharaoh_Terrain.555",
        "data/Pharaoh_Terrain.sg3",
        "data/Pharaoh_Fonts.555",
        "data/Pharaoh_Fonts.sg3",
        "",
        "",
        "data/Empire.555",
        "data/Empire.sg3",
        {
                "data/Assyrian.555",
                "data/Egyptian.555",
                "data/Canaanite.555",
                "data/Enemy_1.555",
                "data/Hittite.555",
                "data/Hyksos.555",
                "data/Kushite.555",
                "data/Libian.555",
                "data/Mitani.555",
                "data/Nubian.555",
                "data/Persian.555",
                "data/Phoenician.555",
                "data/Roman.555",
                "data/SeaPeople.555"
        },
        {
                "data/Assyrian.sg3",
                "data/Egyptian.sg3",
                "data/Canaanite.sg3",
                "data/Enemy_1.sg3",
                "data/Hittite.sg3",
                "data/Hyksos.sg3",
                "data/Kushite.sg3",
                "data/Libian.sg3",
                "data/Mitani.sg3",
                "data/Nubian.sg3",
                "data/Persian.sg3",
                "data/Phoenician.sg3",
                "data/Roman.sg3",
                "data/SeaPeople.sg3"
        },
        "data/Expansion.555",
        "data/Expansion.sg3",
        "data/SprMain.555",
        "data/SprMain.sg3",
        "data/SprMain2.555",
        "data/SprMain2.sg3",
        "data/SprAmbient.555",
        "data/SprAmbient.sg3",
};

game_images::game_images():
    current_climate(-1), editor(false), fonts_enabled(false),
    font_base_offset(0), terrain_ph_offset(0) {
}

const color_t *game_images::load_external_data(image *img) {
    std::string filename = img->get_bitmap_name();
    file_change_extension(filename, EXTENSION_555);

    log_info("Load external image from", filename.c_str(), img->get_data_length());

    // Check root folder first
    buffer buf(img->get_data_length());
    size_t size = io_read_file_part_into_buffer(filename.c_str(), MAY_BE_LOCALIZED, &buf,
                                                img->get_data_length(), img->get_offset() - 1);

    // Try in 555/data dir
    if (!size) {
        std:: string data_filename;
        if (GAME_ENV == ENGINE_ENV_C3) {
            data_filename = DATA_FOLDER_C3 + filename;
        } else if (GAME_ENV == ENGINE_ENV_PHARAOH) {
            data_filename = DATA_FOLDER_PH + filename;
        }

        size = io_read_file_part_into_buffer(data_filename.c_str(),MAY_BE_LOCALIZED, &buf,
                                             img->get_data_length(), img->get_offset() - 1);

        if (!size) {
            log_error("Unable to load external image from data folder", data_filename.c_str(), img->get_data_length());
            return nullptr;
        }
    }

    // NB: isometric images are never external
    auto* external_image_data = new color_t[img->get_data_length()];
    if (img->is_fully_compressed())
        image::convert_compressed(&buf, img->get_data_length(), external_image_data);
    else {
        image::convert_uncompressed(&buf, img->get_data_length(), external_image_data);
    }

    img->set_data(external_image_data, img->get_data_length());
    img->set_external(0);
    delete[] external_image_data;

    return img->get_data();
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
int game_images::get_image_id(int group) {
    switch (GAME_ENV) {
        case ENGINE_ENV_C3:
            return main.get_id(group);
        case ENGINE_ENV_PHARAOH:
            group = image_groupid_translation(groupid_translation_table_ph, group);
//            if (group == GROUP_SYSTEM_GRAPHICS)
//                return 11026;
//            if (group == 1)
//                return 615 + data.ph_terrain->id_shift_overall;
            if (group < 67) {
                return ph_terrain.get_id(group);
            } else if (group < 295) {
                return main.get_id(group - 66);// + 2000;
            } else if (group < 333) {
                return ph_unloaded.get_id(group - 294);// + 5000;
            } else if (group < 341) {
                return font.get_id(group - 332);// + 6000;
            } else if (group < 555) {
                return ph_sprmain.get_id(group - 341);// + 8000;
            } else {
                return ph_sprambient.get_id(group - 555);// + ????;
            }
    }
    return -1;
}

int image_id_from_group(int group) {
    return game_images::get().get_image_id(group);
}

image *game_images::get_image(int id, int mode) {
    image* img = &image::dummy();
    switch (GAME_ENV) {
        case ENGINE_ENV_C3:
            img = main.get_image(id);
            break;
        case ENGINE_ENV_PHARAOH: // todo: mods
            img = ph_expansion.get_image(id);
            if (!img->is_dummy()) {
                return img;
            }

            img = ph_sprmain.get_image(id);
            if (!img->is_dummy()) {
                return img;
            }

            img = ph_unloaded.get_image(id);
            if (!img->is_dummy()){
                return img;
            }

            img = main.get_image(id);
            if (!img->is_dummy()) {
                return img;
            }

            img = ph_terrain.get_image(id);
            if (!img->is_dummy()) {
                return img;
            }

            img = font.get_image(id);
            if (!img->is_dummy()) {
                return img;
            }

            img = ph_sprambient.get_image(id);
            if (!img->is_dummy()) {
                return img;
            }

            img = empire.get_image(id);
            if (!img->is_dummy()) {
                return img;
            }

            break;
    }

    SDL_Log("Image with group id '%d' not found", id);
    return img;
}

image *game_images::get_image(const char* search_tag) {
    image* img = &image::dummy();

    img = ph_expansion.get_image(search_tag);
    if (!img->is_dummy()) {
        return img;
    }

    img = ph_sprmain.get_image(search_tag);
    if (!img->is_dummy()) {
        return img;
    }

    img = ph_unloaded.get_image(search_tag);
    if (!img->is_dummy()){
        return img;
    }

    img = main.get_image(search_tag);
    if (!img->is_dummy()) {
        return img;
    }

    img = ph_terrain.get_image(search_tag);
    if (!img->is_dummy()) {
        return img;
    }

    img = font.get_image(search_tag);
    if (!img->is_dummy()) {
        return img;
    }

    img = ph_sprambient.get_image(search_tag);
    if (!img->is_dummy()) {
        return img;
    }

    img = empire.get_image(search_tag);
    if (!img->is_dummy()) {
        return img;
    }

    SDL_Log("Image with tag '%s' not found", search_tag);
    return img;
}

image *image_get(int id, int mode) {
    return game_images::get().get_image(id, mode);
}

const image *game_images::image_letter(int letter_id) {
    if (fonts_enabled == FULL_CHARSET_IN_FONT) {
        return font.get_image(font_base_offset + letter_id);
    } else if (fonts_enabled == MULTIBYTE_IN_FONT && letter_id >= IMAGE_FONT_MULTIBYTE_OFFSET) {
        return font.get_image(font_base_offset + letter_id - IMAGE_FONT_MULTIBYTE_OFFSET);
    } else if (letter_id < IMAGE_FONT_MULTIBYTE_OFFSET) {
        return get_image(get_image_id(GROUP_FONT) + letter_id);
    }

    return &image::dummy();
}

const image *image_letter(int letter_id) {
    return game_images::get().image_letter(letter_id);
}

const image *game_images::image_get_enemy(int id) {
    return enemy.get_image(id);
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
    if (img->is_external()) {
        return load_external_data(img); // TODO: move to image collection class ?
    } else {
        return img->get_data(); // todo: mods
    }
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

int game_images::load_main(int climate_id, int is_editor, int force_reload) {
    if (climate_id == current_climate && !force_reload) {
        return 1;
    }

    const char *filename_555;
    const char *filename_sgx;
    switch (GAME_ENV) {
        case ENGINE_ENV_C3:
            filename_555 = is_editor ? gfc.C3_EDITOR_555[climate_id] : gfc.C3_MAIN_555[climate_id];
            filename_sgx = is_editor ? gfc.C3_EDITOR_SG2[climate_id] : gfc.C3_MAIN_SG2[climate_id];
            if (!main.load_files(filename_555, filename_sgx)) {
                return 0;
            }
            break;
        case ENGINE_ENV_PHARAOH:
            filename_555 = is_editor ? gfc.PH_EDITOR_GRAPHICS_555 : gfc.PH_MAIN_555;
            filename_sgx = is_editor ? gfc.PH_EDITOR_GRAPHICS_SG3 : gfc.PH_MAIN_SG3;
            if (!ph_expansion.load_files(gfc.PH_EXPANSION_555, gfc.PH_EXPANSION_SG3, -200)) {
                return 0;
            }
            if (!ph_sprmain.load_files(gfc.PH_SPRMAIN_555, gfc.PH_SPRMAIN_SG3, 700)) {
                return 0;
            }
            if (!ph_unloaded.load_files(gfc.PH_UNLOADED_555, gfc.PH_UNLOADED_SG3, 11025)) {
                return 0;
            }
            if (!main.load_files(filename_555, filename_sgx, 11706)) {
                return 0;
            }
            // ???? 539-long gap?
            if (!ph_terrain.load_files(gfc.PH_TERRAIN_555, gfc.PH_TERRAIN_SG3, 14252)) {
                return 0;
            }
            // ???? 64-long gap?
            if (!ph_sprambient.load_files(gfc.PH_SPRAMBIENT_555, gfc.PH_SPRAMBIENT_SG3, 15766+64)) {
                return 0;
            }
            if (!font.load_files(gfc.PH_FONTS_555, gfc.PH_FONTS_SG3, 18764)) {
                return 0;
            }
            if (!empire.load_files(gfc.PH_EMPIRE_555, gfc.PH_EMPIRE_SG3, 0)) {
                return 0;
            }
            break;
    }

    current_climate = climate_id;
    is_editor = is_editor;
    return 1;
}

int game_images::load_enemy(int enemy_id) {
    const char *filename_555;
    const char *filename_sgx;
    switch (GAME_ENV) {
        case ENGINE_ENV_C3:
            filename_555 = gfc.C3_ENEMY_555[enemy_id];
            filename_sgx = gfc.C3_ENEMY_SG2[enemy_id];
            break;
        case ENGINE_ENV_PHARAOH:
            filename_555 = gfc.PH_ENEMY_555[enemy_id];
            filename_sgx = gfc.PH_ENEMY_SG2[enemy_id];
            break;
    }

    if (!enemy.load_files(filename_555, filename_sgx)) {
        return 0;
    }

    return 1;
}

int game_images::load_fonts(encoding_type encoding) {
    int result = 1;
    if (encoding == ENCODING_CYRILLIC) {
        result = 0;
    } else if (encoding == ENCODING_TRADITIONAL_CHINESE) {
        result = 0;
    } else if (encoding == ENCODING_SIMPLIFIED_CHINESE) {
        result = 0;
    } else if (encoding == ENCODING_KOREAN) {
        result = 0;
    } else {
        fonts_enabled = NO_EXTRA_FONT;
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