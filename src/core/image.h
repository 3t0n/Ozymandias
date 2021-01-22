#ifndef CORE_IMAGE_H
#define CORE_IMAGE_H

#include "core/encoding.h"
#include "core/buffer.h"
#include "core/image_group.h"
#include "graphics/color.h"

#include <vector>
#include <string>
#include <algorithm>

#define IMAGE_FONT_MULTIBYTE_OFFSET 10000
#define IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS 2188
#define IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS 2130
#define IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS 2350

enum {
    IMAGE_TYPE_WITH_TRANSPARENCY = 0,
    IMAGE_TYPE_ISOMETRIC = 30,
    IMAGE_TYPE_MOD = 40
};


/**
 * Image class
 */
class image {
private:
    size_t width = 0;
    size_t height = 0;
    int num_animation_sprites = 0;
    int sprite_offset_x = 0;
    int sprite_offset_y = 0;
    int animation_can_reverse = 0;
    int animation_speed_id = 0;
    int offset_mirror = 0;
    int type = 0;
    int fully_compressed = 0;
    int external = 0;
    int compressed_part = 0;
    size_t bmp_index = 0;
    int offset = 0;
    size_t data_length = 0;
    size_t uncompressed_length = 0;
    size_t full_length = 0;
    std::string bitmap_name;
    std::vector<color_t> data;

public:
    image() = default;
    ~image() = default;

    void set_data(color_t *image_data, size_t size);
    const color_t *get_data() const;

    const char *get_name() const;
    void set_name(const char *filename);
    void set_name(const char *filename, size_t size);

    int get_width() const;
    void set_width(int width);
    int get_height() const;
    void set_height(int height);
    int get_type() const;
    void set_type(int type);
    int is_fully_compressed() const;
    void set_fully_compressed(int new_is_fully_compressed);
    int has_compressed_part() const;
    void set_compressed_part(int new_has_compressed_part);
    int get_offset() const;
    void set_offset(int new_offset);
    size_t get_data_length() const;
    void set_data_length(int new_data_length);
    size_t get_uncompressed_length() const;
    void set_uncompressed_length(int new_uncompressed_length);
    size_t get_full_length() const;
    void set_full_length(int new_full_length);
    size_t get_bmp_index() const;
    void set_bmp_index(int new_bmp_index);
    int is_external() const;
    void set_external(int new_external);
    int get_num_animation_sprites() const;
    void set_num_animation_sprites(int new_num_animation_sprites);
    int get_sprite_offset_x() const;
    void set_sprite_offset_x(int new_sprite_offset_x);
    int get_sprite_offset_y() const;
    void set_sprite_offset_y(int new_sprite_offset_y);
    int get_animation_can_reverse() const;
    void set_animation_can_reverse(int new_animation_can_reverse);
    int get_animation_speed_id() const;
    void set_animation_speed_id(int new_animation_speed_id);
    int get_offset_mirror() const;
    void set_offset_mirror(int new_offset_mirror);
};

extern int terrain_ph_offset;

int image_load_main(int climate_id, int is_editor, int force_reload);
int image_load_fonts(encoding_type encoding);
int image_load_enemy(int enemy_id);

int image_id_from_group(int group);

color_t to_32_bit(uint16_t c);
int convert_uncompressed(buffer *buf, int amount, color_t *dst);
int convert_compressed(buffer *buf, int amount, color_t *dst);

const image *image_get(int id, int mode = 0);
const image *image_letter(int letter_id);
const image *image_get_enemy(int id);
const color_t *image_data(int id);
const color_t *image_data_letter(int letter_id);
const color_t *image_data_enemy(int id);

#endif // CORE_IMAGE_H
