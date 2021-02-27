#ifndef GAME_IMAGES_H
#define GAME_IMAGES_H

#include "core/image.h"
#include "core/image_collection.h"

#include <vector>

static const char *FONTS_FILENAMES_C3[] = {
        "C3_fonts",
        "rome",
        "korean",
};

static const char *FONTS_FILENAMES_PH[] = {
        "Pharaoh_Fonts",
};

static const char *ENEMY_FILENAMES_C3[] = {
        "goths",
        "Etruscan",
        "Etruscan",
        "carthage",
        "Greek",
        "Greek",
        "egyptians",
        "Persians",
        "Phoenician",
        "celts",
        "celts",
        "celts",
        "Gaul",
        "Gaul",
        "goths",
        "goths",
        "goths",
        "Phoenician",
        "North African",
        "Phoenician"
};

static const char *ENEMY_FILENAMES_PH[] = {
        "Assyrian",
        "Egyptian",
        "Canaanite",
        "Enemy_1",
        "Hittite",
        "Hyksos",
        "Kushite",
        "Libian",
        "Mitani",
        "Nubian",
        "Persian",
        "Phoenician",
        "Roman",
        "SeaPeople",
};

static const char* MAIN_FILENAME_C3 = "c3";
static const char* EDITOR_FILENAME_C3 = "c3map";
static const char* EMPIRE_FILENAME_C3 = "The_empire";

static const char* MAIN_FILENAME_PH = "Pharaoh_General";
static const char* UNLOADED_FILENAME_PH = "Pharaoh_Unloaded";
static const char* TERRAIN_FILENAME_PH = "Pharaoh_Terrain";
static const char* EDITOR_GRAPHICS_FILENAME_PH = "FE_Map Editor"; // TODO: check map editor pack
static const char* EMPIRE_FILENAME_PH = "Empire";
static const char* EXPANSION_FILENAME_PH = "Expansion";
static const char* SPRMAIN_FILENAME_PH = "SprMain";
static const char* SPRMAIN2_FILENAME_PH = "SprMain2";
static const char* SPRAMBIENT_FILENAME_PH = "SprAmbient";
static const char* MASTABA_FILENAME_PH = "mastaba";
static const char* TEMPLE_BAST_FILENAME_PH = "Temple_bast";
static const char* TEMPLE_NILE_FILENAME_PH = "Temple_nile";
static const char* TEMPLE_PTAH_FILENAME_PH = "Temple_ptah";
static const char* TEMPLE_RA_FILENAME_PH = "Temple_ra";
static const char* TEMPLE_SETH_FILENAME_PH = "Temple_seth";


class game_images {
private:
    bool editor;
    bool fonts_enabled;
    encoding_type font_encoding;
    int32_t current_climate;
    int32_t font_base_offset;
    int32_t terrain_ph_offset;
    int32_t enemy_id;

    // Sequence is important here for getting correct group_id
    // shift is needed to support getting correct image_ids from different collections
    std::vector<image_collection> collections = {
            // TODO: fix terrain offset, because it hardcoded somewhere
            {TERRAIN_FILENAME_PH, 14252}, // 66 groups 1516 images
            {MAIN_FILENAME_PH, 16000}, // 294 groups 2747 images
            {UNLOADED_FILENAME_PH, 19000}, // 332 groups 683 images
            {FONTS_FILENAMES_PH[0], 20000}, // 339 groups 1541 images
            {SPRMAIN_FILENAME_PH, 22000}, // 553 groups 10326 images
            {SPRAMBIENT_FILENAME_PH, 33000}, // 2935 images
            {ENEMY_FILENAMES_PH[0], 36000}, // 900 images
            {EXPANSION_FILENAME_PH, 37000},
            {EMPIRE_FILENAME_PH, 38000},
            {SPRMAIN2_FILENAME_PH, 39000},
            {MASTABA_FILENAME_PH, 41000},
            {TEMPLE_BAST_FILENAME_PH, 42000},
            {TEMPLE_NILE_FILENAME_PH, 43000},
            {TEMPLE_PTAH_FILENAME_PH, 44000},
            {TEMPLE_RA_FILENAME_PH, 45000},
            {TEMPLE_SETH_FILENAME_PH, 46000},
    };

    image_collection& get_enemy();
    image_collection& get_font();
    const image_collection& get_collection(const char* collection_name);
    const image_collection& get_collection(std::string& collection_name);

public:
    game_images();
    game_images(game_images const&) = delete;             // Copy construct
    game_images(game_images&&) = delete;

    // Move construct
    game_images& operator=(game_images const&) = delete;  // Copy assign
    game_images& operator=(game_images &&) = delete;      // Move assign
    ~game_images() = default;

    // singleton
    static game_images& get() {
        static game_images instance;
        return instance;
    }

    // load images from files
    bool load_main(int climate_id, int is_editor, int force_reload);
    bool load_fonts(encoding_type encoding);
    bool load_enemy(int enemy_id);

    // getting images
    int32_t get_image_id(int group);
    image *get_image(int id);
    image *get_image(const char* search_tag);
    const image *image_letter(int letter_id);
    const image *image_get_enemy(int id);
    const color_t *image_data(int id);
    const color_t *image_data(image *img);
    const color_t *image_data_letter(int letter_id);
    const color_t *image_data_enemy(int id);
    static int image_groupid_translation(int *table, int group);

    // getting collection shift to set proper image/group ids
    size_t get_images_shift(std::string& collection_name);
    size_t get_groups_shift(std::string& collection_name);

    // getters & setters
    int get_current_climate() const;
    void set_current_climate(int new_climate);
    bool is_editor() const;
    void set_editor(bool new_editor);
    bool is_fonts_enabled() const;
    void set_fonts_enabled(bool new_fonts_enabled);
    int get_font_base_offset() const;
    void set_font_base_offset(int new_font_base_offset);
    int get_terrain_ph_offset() const;
    void set_terrain_ph_offset(int new_terrain_ph_offset);

    // Prints class information
    void print();
};

// TODO: temporary functions to be removed
int image_id_from_group(int group);
image *image_get(int id);
const image *image_letter(int letter_id);
const image *image_get_enemy(int id);
const color_t *image_data(int id);
const color_t *image_data(image* img);
const color_t *image_data_letter(int letter_id);
const color_t *image_data_enemy(int id);

#endif //GAME_IMAGES_H
