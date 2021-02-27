#include "SDL.h"
#include "core/buffer.h"
#include "core/file.h"
#include "core/io.h"
#include "core/log.h"
#include "core/mods.h"
#include "core/image_collection.h"
#include "core/game_environment.h"

image_collection::image_collection(std::string new_filename, int32_t new_shift) {
    set_filename(new_filename);
    set_shift(new_shift);
}

color_t image_collection::to_32_bit(uint16_t c) {
    return ALPHA_OPAQUE |
           ((c & 0x7c00) << 9) | ((c & 0x7000) << 4) |
           ((c & 0x3e0) << 6) | ((c & 0x380) << 1) |
           ((c & 0x1f) << 3) | ((c & 0x1c) >> 2);
}

int32_t image_collection::convert_uncompressed(buffer *buf, int32_t amount, color_t *dst) {
    for (int i = 0; i < amount; i += 2) {
        color_t c = to_32_bit(buf->read_u16());
        *dst = c;
        dst++;
    }
    return amount / 2;
}

int32_t image_collection::convert_compressed(buffer *buf, int32_t amount, color_t *dst) {
    int dst_length = 0;
    while (amount > 0) {
        int control = buf->read_u8();
        if (control == 255) {
            // next byte = transparent pixels to skip
            *dst++ = 255;
            *dst++ = buf->read_u8();
            dst_length += 2;
            amount -= 2;
        } else {
            // control = number of concrete pixels
            *dst++ = control;
            for (int i = 0; i < control; i++) {
                *dst++ = to_32_bit(buf->read_u16());
            }
            dst_length += control + 1;
            amount -= control * 2 + 1;
        }
    }
    return dst_length;
}

int32_t image_collection::convert(const image *img, buffer &buffer, color_t* dst) {
    int32_t image_size = 0;

    // NB: isometric images are never external
    if (img->is_fully_compressed()) {
        image_size = convert_compressed(&buffer, img->get_data_length(), dst);
    } else if (img->has_compressed_part()) { // isometric tile
        size_t uncompressed_size = convert_uncompressed(&buffer, img->get_uncompressed_length(), dst);
        size_t compressed_size = convert_compressed(&buffer, img->get_data_length() - img->get_uncompressed_length(), dst + uncompressed_size);

        image_size = uncompressed_size + compressed_size;
    } else {
        image_size = convert_uncompressed(&buffer, img->get_data_length(), dst);
    }
    return image_size;
}

bool image_collection::is_dummy() const {
    return (this == &dummy());
}

bool image_collection::load_sgx() {
    // prepare sgx data
    size_t file_size = io_get_file_size(get_filename_sgx());
    SDL_Log("Loading image collection from file '%s': %zu", get_filename_sgx(), file_size);
    if (!file_size) {
        SDL_Log("Loading image collection from file '%s': empty file", get_filename_sgx());
        return false;
    }

    buffer buffer_sgx(file_size);
    if (!io_read_file_into_buffer(get_filename_sgx(), MAY_BE_LOCALIZED, &buffer_sgx, MAX_FILE_SIZE)) { //int MAIN_INDEX_SIZE = 660680;
        SDL_Log("Loading image collection from file '%s': can't read file", get_filename_sgx());
        return false;
    }
    
    size_t header_size = HEADER_SG3_SIZE;
    if (file_has_extension(get_filename_sgx(), "sg2")) {
        header_size = HEADER_SG2_SIZE; // sg2 has 100 bitmap entries
    } else if (file_has_extension(get_filename_sgx(), "sg3")) {
        header_size = HEADER_SG3_SIZE;
    } else {
        SDL_Log("Loading image collection from file '%s': wrong extension", get_filename_sgx());
        return false;
    }

    // read header
    sgx_filesize = buffer_sgx.read_u32();
    sgx_version = buffer_sgx.read_u32();
    unknown1 = buffer_sgx.read_u32();
    max_image_records = buffer_sgx.read_i32();
    // First image is empty, so actual number of images +1
    num_image_records = buffer_sgx.read_i32() + 1;
    num_bitmap_records = buffer_sgx.read_i32();
    unknown2 = buffer_sgx.read_i32();
    total_filesize = buffer_sgx.read_u32();
    filesize_555 = buffer_sgx.read_u32();
    filesize_external = buffer_sgx.read_u32();

    // allocate arrays
    images.reserve(num_image_records);

    buffer_sgx.skip(40); // skip remaining 40 bytes

    // parse groups (always a fixed 300 pool)
    // added a zero item, because counting from one
    group_image_ids.push_back(0);
    for (size_t i = 0; i < GROUP_IMAGE_IDS_SIZE; i++) {
        auto image_id = buffer_sgx.read_u16();
        if (image_id != 0) {
            group_image_ids.push_back(image_id);
//            SDL_Log("%s group %i -> id %i", filename_sgx, i, group_image_ids[i]);
        }
    }

    // parse bitmap names
    // every line is 200 chars - 97 entries in the original c3.sg2 header (100 for good measure) and 18 in Pharaoh_General.sg3
    for (size_t i = 0; i < num_bitmap_records; i++) {
        char folder_name[BMP_NAME_SIZE] = {};
        char folder_comment[BMP_COMMENT_SIZE] = {};
        buffer_sgx.read_raw(folder_name, BMP_NAME_SIZE);
        buffer_sgx.read_raw(folder_comment, BMP_COMMENT_SIZE);
        bitmap_image_names.emplace_back(folder_name);
        bitmap_image_comments.emplace_back(folder_comment);
//        SDL_Log("%s folder %i: '%s' -> '%s'", filename_sgx, i, folder_name, folder_comment);
    }

    // move on to the rest of the content
    buffer_sgx.set_offset(header_size);

    // fill in image data
    int bmp_lastbmp = 0;
    int bmp_lastindex = 1;
    for (size_t i = 0; i < num_image_records; i++) {
        image img;
        img.set_collection(this);
        img.set_absolute_index(i);
        img.set_offset(buffer_sgx.read_i32());
        img.set_data_length(buffer_sgx.read_i32());
        img.set_uncompressed_length(buffer_sgx.read_i32());
        buffer_sgx.skip(4);
        img.set_offset_mirror(buffer_sgx.read_i32()); // .sg3 only
        img.set_width(buffer_sgx.read_u16());
        img.set_height(buffer_sgx.read_u16());
        buffer_sgx.skip(6);
        img.set_num_animation_sprites(buffer_sgx.read_u16());
        buffer_sgx.skip(2);
        img.set_sprite_offset_x(buffer_sgx.read_i16());
        img.set_sprite_offset_y(buffer_sgx.read_i16());
        buffer_sgx.skip(10);
        img.set_animation_can_reverse(buffer_sgx.read_i8());
        buffer_sgx.skip(1);
        img.set_type(buffer_sgx.read_u8());
        img.set_fully_compressed(buffer_sgx.read_i8());
        img.set_external(buffer_sgx.read_i8());
        img.set_compressed_part(buffer_sgx.read_i8());
        buffer_sgx.skip(2);
        uint8_t bitmap_id = buffer_sgx.read_u8();
        img.set_bitmap_index(bitmap_id);
        img.set_bitmap_name(bitmap_image_names.at(bitmap_id).c_str());
        // new bitmap name, reset bitmap grouping index
        if (bitmap_id != bmp_lastbmp) {
            bmp_lastindex = 1;
            bmp_lastbmp = bitmap_id;
        }
        img.set_index(bmp_lastindex);
        bmp_lastindex++;
        buffer_sgx.skip(1);
        img.set_animation_speed_id(buffer_sgx.read_u8());
        buffer_sgx.skip(5);
        // Read alphas for 0xD6 SG3 versions only
        if (get_sgx_version() >= 0xd6) {
            img.set_alpha_offset(buffer_sgx.read_u32());
            img.set_alpha_length(buffer_sgx.read_u32());
        }
        images.emplace_back(img);
    }

    // fill in bmp offset data
    int offset = 4;
    for (size_t i = 1; i < num_image_records; i++) {
        image *img = &images.at(i);
        if (img->is_external()) {
            if (!img->get_offset()) {
                img->set_offset(1);
            }
        } else {
            img->set_offset(offset);
            offset += img->get_data_length();
        }
    }

    // go to the end to get group names for sg3
    // always 299 x 48 bytes = 14352
    // add one empty to sync with GROUP_IMAGE_IDS
    group_image_tags.emplace_back("");
    if (get_sgx_version() >= 0xd5) {
        buffer_sgx.set_offset(buffer_sgx.size() - IMAGE_TAGS_OFFSET);
        for (size_t i = 0; i < get_num_group_records() - 1; i++) {
            char group_tag[GROUP_IMAGE_TAG_SIZE] = {};
            buffer_sgx.read_raw(group_tag, GROUP_IMAGE_TAG_SIZE);
            group_image_tags.emplace_back(group_tag);
//            SDL_Log("%s tag %i: '%s'", filename_sgx, i, group_tag);
        }
    }

    SDL_Log("Loaded  image collection from file '%s': %d images and %d image groups",
            get_filename_sgx(), get_num_image_records(), get_num_group_records());

    return true;
}

bool image_collection::load_555() {
    // prepare bitmap data
    size_t file_size = io_get_file_size(get_filename_555());
    SDL_Log("Loading image collection from file '%s': %zu", get_filename_555(), file_size);
    if (!file_size) {
        SDL_Log("Loading image collection from file '%s': empty file", get_filename_555());
        return false;
    }

    buffer buffer_555(file_size);
    int data_size = io_read_file_into_buffer(get_filename_555(), MAY_BE_LOCALIZED, &buffer_555, MAX_FILE_SIZE);
    if (!data_size) {
        SDL_Log("Loading image collection from file '%s': can't read file", get_filename_555());
        return false;
    }

    // temp variable for image data
    auto *data = new color_t[num_image_records * MAX_IMAGE_SIZE];

    // counters
    size_t count_images = 0;
    size_t count_external = 0;

    // convert bitmap data for image pool
    color_t *start_dst = data;
    color_t *dst = data;
    dst++; // make sure img->offset > 0
    for (size_t i = 0; i < num_image_records; i++) {
        image *img = &images.at(i);
        // if external, image will automatically loaded in the runtime
        if (img->is_external()) {
            count_external++;
            continue;
        }
        buffer_555.set_offset(img->get_offset());
        int img_offset = (int) (dst - start_dst);
        size_t image_size = convert(img, buffer_555, dst);
        dst += image_size;

        img->set_offset(img_offset);
        img->set_uncompressed_length(img->get_uncompressed_length()/2);
        img->set_full_length(image_size);
        img->set_data(&data[img_offset], image_size);

        count_images++;
    }

    SDL_Log("Loaded  image collection from file '%s': %zu images and %zu externals",
            get_filename_555(), count_images, count_external);

    delete[] data;
    return true;
}

bool image_collection::load_files() {
    return load_sgx() && load_555();
}

const color_t *image_collection::load_external(image *img) const {
    std::string external_filename = img->get_bitmap_name();
    file_change_extension(external_filename, extension_555);

    log_info("Load external image from", external_filename.c_str(), img->get_data_length());

    // Check root folder first
    buffer buf(img->get_data_length());
    size_t size = io_read_file_part_into_buffer(external_filename.c_str(), MAY_BE_LOCALIZED, &buf,
                                                img->get_data_length(), img->get_offset() - 1);

    // Try in 555/data dir
    if (!size) {
        std::string data_filename = data_folder + "/" + external_filename;

        size = io_read_file_part_into_buffer(data_filename.c_str(),MAY_BE_LOCALIZED, &buf,
                                             img->get_data_length(), img->get_offset() - 1);

        if (!size) {
            log_error("Unable to load external image from data folder", data_filename.c_str(), img->get_data_length());
            return nullptr;
        }
    }

    auto* external_image_data = new color_t[img->get_data_length()];
    convert(img, buf, external_image_data);

    img->set_data(external_image_data, img->get_data_length());
    img->set_external(0);

    delete[] external_image_data;
    return img->get_data();
}

int32_t image_collection::get_num_image_records() const {
    return images.size();
}

uint32_t image_collection::get_num_group_records() const {
    return group_image_ids.size();
}

int32_t image_collection::get_id(int group_id) const {
    int32_t result = -1;
    if (group_id >= 0 && group_id < group_image_ids.size()) {
        result = group_image_ids.at(group_id) + get_shift();
    } else {
        SDL_Log("Wrong group index for collection: '%s': group_id: %d, expected > 0 and < %zu", get_filename(), group_id, group_image_ids.size());
    }
    return result;
}

image *image_collection::get_image(int id, bool relative) {
    if (!relative) {
        id -= get_shift();
    }
    if (id < 0 || id >= num_image_records) {
//        SDL_Log("Wrong image index: %d, expected < %d, relative:%d", id, num_image_records, relative);
        return &image::dummy();
    }
    return &images.at(id);
}

image *image_collection::get_image(const char *search_tag) {
    auto result = &image::dummy();
    for (size_t i = 0; i < group_image_tags.size(); ++i) {
        if (group_image_tags.at(i) == search_tag) {
            auto image_id = group_image_ids.at(i);
            result = &images.at(image_id);
            break;
        }
    }
    return result;
}

int32_t image_collection::get_shift() const {
    return id_shift_overall;
}

void image_collection::set_shift(int32_t shift) {
    id_shift_overall = shift;
}

const char *image_collection::get_filename() const {
    return filename.c_str();
}

void image_collection::set_filename(std::string& new_filename) {
    filename = new_filename;
    extension_555 = EXTENSION_555;

    if (GAME_ENV == ENGINE_ENV_C3) {
        data_folder = DATA_FOLDER_C3;
        extension_sgx = EXTENSION_SG2;
        filename_sgx = filename + "." + extension_sgx;
    } else if (GAME_ENV == ENGINE_ENV_PHARAOH) {
        data_folder = DATA_FOLDER_PH;
        extension_sgx = EXTENSION_SG3;
        filename_sgx = data_folder + "/" + filename + "." + extension_sgx;
    }

    filename_555 = data_folder + "/" + filename + "." + extension_555;
}

const char *image_collection::get_filename_sgx() const {
    return filename_sgx.c_str();
}

const char *image_collection::get_filename_555() const {
    return filename_555.c_str();
}

uint32_t image_collection::get_sgx_version() const {
    return sgx_version;
}

const char *image_collection::get_data_folder() const {
    return data_folder.c_str();
}

const char *image_collection::get_extension_sgx() const {
    return extension_sgx.c_str();
}

const char *image_collection::get_extension_555() const {
    return extension_555.c_str();
}

image *image_collection::get_image_by_group(int group_id) {
    return get_image(get_id(group_id));
}

void image_collection::print() {
    SDL_Log("Collection filename: '%s', number of images %d", get_filename_sgx(), get_num_image_records());

    for (size_t i = 0; i < bitmap_image_names.size(); i++) {
        SDL_Log("Bitmap name: '%s', comment: '%s'",
                bitmap_image_names.at(i).c_str(), bitmap_image_comments.at(i).c_str());

        // check all images for corresponding index
        for (size_t j = 0; j < group_image_ids.size(); j++) {
            auto image_id = group_image_ids.at(j);
            image &image = images.at(image_id);
            if (i == image.get_bitmap_index()) {
                SDL_Log("Group: tag: '%s', id: %zu", group_image_tags.at(j).c_str(), j);

                // print all images related to this group
                auto num_images = image.get_num_animation_sprites();
                for (size_t z = image_id; z <= image_id + num_images; z++) {
                    images.at(z).print();
                }
            }
        }
    }
}