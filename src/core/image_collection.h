#ifndef IMAGE_COLLECTION_H
#define IMAGE_COLLECTION_H

#include "core/encoding.h"
#include "core/image.h"
#include "core/image_group.h"
#include "core/buffer.h"
#include "core/file.h"
#include "graphics/color.h"

#include <vector>
#include <string>

class image_collection {
private:
    static const size_t BMP_NAME_SIZE = 65;
    static const size_t BMP_COMMENT_SIZE = 135;
    static const size_t GROUP_IMAGE_IDS_SIZE = 300;
    static const size_t GROUP_IMAGE_TAG_SIZE = 48;
    static const size_t MAX_FILE_SIZE = 20000000;
    static const size_t MAX_IMAGE_SIZE = 10000;
    static const size_t HEADER_SG2_SIZE = 20680;
    static const size_t HEADER_SG3_SIZE = 40680;
    static const size_t IMAGE_TAGS_OFFSET = 14352;

    uint32_t sgx_filesize = 0;
    uint32_t sgx_version = 0;
    uint32_t unknown1 = 0;
    int32_t max_image_records = 0;
    int32_t num_image_records = 0;
    int32_t num_bitmap_records = 0;
    int32_t num_bitmap_records_without_system = 0; /* ? */
    uint32_t total_filesize = 0;
    uint32_t filesize_555 = 0;
    uint32_t filesize_external = 0;

    int32_t id_shift_overall = 0;
    std::string name;
    size_t num_groups_records = 0;
    std::vector<uint16_t> group_image_ids;
    std::vector<std::string> group_image_tags;
    std::vector<image> images;

public:
    image_collection() = default;
    ~image_collection() = default;

    int load_sgx(const char *filename_sgx, int shift = 0);
    int load_555(const char *filename_555);
    int load_files(const char *filename_555, const char *filename_sgx, int shift = 0);

    int32_t get_shift() const;
    void set_shift(int32_t shift);
    const char *get_name() const;
    void set_name(const char *filename);

    int size() const;
    int get_id(int group);
    image *get_image(int id, bool relative = false);
    uint32_t get_sgx_version() const;
    void print();
};

#endif //IMAGE_COLLECTION_H
