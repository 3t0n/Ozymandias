#include "SDL.h"
#include "core/image.h"
#include "core/file.h"
#include "core/io.h"
#include "core/image_collection.h"

void image::set_data(color_t *image_data, size_t size) {
    data.reserve(size);
    std::copy(image_data, image_data + size, std::back_inserter(data));
}

const color_t *image::image::get_data() const {
    if (is_external() && data.empty()) {
        collection->load_external(const_cast<image *>(this));
    }

    return data.data();
}

const char *image::get_bitmap_name() const {
    return bitmap_name.c_str();
}

void image::set_bitmap_name(const char *filename) {
    bitmap_name = std::string(filename);
}

void image::set_bitmap_name(const char *filename, size_t size) {
    bitmap_name = std::string(filename, size);
}

uint16_t image::get_width() const {
    return width;
}

void image::set_width(uint16_t new_width) {
    width = new_width;
}

uint16_t image::get_height() const {
    return height;
}

void image::set_height(uint16_t new_height) {
    height = new_height;
}

uint8_t image::get_type() const {
    return type;
}

void image::set_type(uint8_t new_type) {
    type = new_type;
}

uint8_t image::is_fully_compressed() const {
    return fully_compressed;
}

void image::set_fully_compressed(uint8_t new_fully_compressed) {
    fully_compressed = new_fully_compressed;
}

uint8_t image::has_compressed_part() const {
    return compressed_part;
}

void image::set_compressed_part(uint8_t new_compressed_part) {
    compressed_part = new_compressed_part;
}

int32_t image::get_offset() const {
    return offset;
}

void image::set_offset(int32_t new_offset) {
    offset = new_offset;
}

int32_t image::get_data_length() const {
    return data_length;
}

void image::set_data_length(int32_t new_data_length) {
    data_length = new_data_length;
}

int image::is_external() const {
    return external;
}

void image::set_external(int new_external) {
    external = new_external;
}

int32_t image::get_uncompressed_length() const {
    return uncompressed_length;
}

void image::set_uncompressed_length(int32_t new_uncompressed_length) {
    uncompressed_length = new_uncompressed_length;
}

int32_t image::get_full_length() const {
    return full_length;
}

void image::set_full_length(int32_t new_full_length) {
    full_length = new_full_length;
}

uint8_t image::get_index() const {
    return index;
}

void image::set_index(uint8_t new_bmp_index) {
    index = new_bmp_index;
}

uint16_t image::get_num_animation_sprites() const {
    return num_animation_sprites;
}

void image::set_num_animation_sprites(uint16_t new_num_animation_sprites) {
    num_animation_sprites = new_num_animation_sprites;
}

int16_t image::get_sprite_offset_x() const {
    return sprite_offset_x;
}

void image::set_sprite_offset_x(int16_t new_sprite_offset_x) {
    sprite_offset_x = new_sprite_offset_x;
}

int16_t image::get_sprite_offset_y() const {
    return sprite_offset_y;
}

void image::set_sprite_offset_y(int16_t new_sprite_offset_y) {
    sprite_offset_y = new_sprite_offset_y;
}

int8_t image::get_animation_can_reverse() const {
    return animation_can_reverse;
}

void image::set_animation_can_reverse(int8_t new_animation_can_reverse) {
    animation_can_reverse = new_animation_can_reverse;
}

uint8_t image::get_animation_speed_id() const {
    return animation_speed_id;
}

void image::set_animation_speed_id(uint8_t new_animation_speed_id) {
    animation_speed_id = new_animation_speed_id;
}

int32_t image::get_offset_mirror() const {
    return offset_mirror;
}

void image::set_offset_mirror(int32_t new_offset_mirror) {
    offset_mirror = new_offset_mirror;
}

bool image::is_dummy() const {
    return (this == &dummy());
}

uint32_t image::get_alpha_offset() const {
    return alpha_offset;
}

void image::set_alpha_offset(uint32_t new_alpha_offset) {
    alpha_offset = new_alpha_offset;
}

uint32_t image::get_alpha_length() const {
    return alpha_length;
}

void image::set_alpha_length(uint32_t new_alpha_length) {
    alpha_length = new_alpha_length;
}

uint8_t image::get_bitmap_index() const {
    return bitmap_index;
}

void image::set_bitmap_index(uint8_t new_bmp_index) {
    bitmap_index = new_bmp_index;
}

uint32_t image::get_absolute_index() const {
    return absolute_index;
}

void image::set_absolute_index(uint32_t new_abs_index) {
    absolute_index = new_abs_index;
}

const image_collection *image::get_collection() const {
    return collection;
}

void image::set_collection(const image_collection *new_collection) {
    collection = new_collection;
}

void image::print() const {
    SDL_Log("Image id: %d, abs_id: %d, type: %u, ext: %d, fcompr: %d, width: %u, height: %u, size: %u",
            get_index(), get_absolute_index(), get_type(), is_external(), is_fully_compressed(), get_width(), get_height(), get_full_length());
}