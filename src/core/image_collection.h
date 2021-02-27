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

// Image collection class that represents SGX and 555 files
// SGX file -> image folders/bitmaps -> image groups/sprites -> images
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

    const char* DATA_FOLDER_C3 = "555";
    const char* DATA_FOLDER_PH = "Data";
    const char* EXTENSION_555 = "555";
    const char* EXTENSION_SG2 = "sg2";
    const char* EXTENSION_SG3 = "sg3";

    // filenames & path
    std::string data_folder;
    std::string extension_sgx;
    std::string extension_555;
    std::string filename;
    std::string filename_sgx;
    std::string filename_555;

    // sgx header
    uint32_t sgx_filesize = 0;
    uint32_t sgx_version = 0;
    uint32_t unknown1 = 0;
    int32_t max_image_records = 0;
    int32_t num_image_records = 0;
    int32_t num_bitmap_records = 0;
    int32_t unknown2 = 0;
    uint32_t total_filesize = 0;
    uint32_t filesize_555 = 0;
    uint32_t filesize_external = 0;
    int32_t id_shift_overall = 0;

    // image groups data
    std::vector<uint16_t> group_image_ids;
    std::vector<std::string> group_image_tags;

    // image bitmaps/folders/collections
    std::vector<std::string> bitmap_image_names;
    std::vector<std::string> bitmap_image_comments;

    // 555 image data
    std::vector<image> images;

    // methods for loading images
    static color_t to_32_bit(uint16_t c);
    static int32_t convert_uncompressed(buffer *buf, int32_t amount, color_t *dst);
    static int32_t convert_compressed(buffer *buf, int32_t amount, color_t *dst);
    static int32_t convert(const image *img, buffer &buffer, color_t *dst) ;

public:
    image_collection() = delete;
    image_collection(std::string new_filename, int32_t new_shift);
    ~image_collection() = default;

    // dummy collection
    static image_collection& dummy() {
        static image_collection dummy = {"", 0};
        return dummy;
    }
    bool is_dummy() const;

    bool load_sgx();
    bool load_555();
    bool load_files();
    const color_t *load_external(image *img) const;

    int32_t get_shift() const;
    void set_shift(int32_t shift);

    void set_filename(std::string& new_filename);
    const char *get_filename() const;
    const char *get_filename_sgx() const;
    const char *get_filename_555() const;
    const char *get_data_folder() const;
    const char *get_extension_sgx() const;
    const char *get_extension_555() const;


    int32_t get_num_image_records() const;
    uint32_t get_num_group_records() const;
    int32_t get_id(int group_id) const;
    image *get_image(int id, bool relative = false);
    image *get_image(const char* group_tag);
    image *get_image_by_group(int group_id);
    uint32_t get_sgx_version() const;

    void print();
};

#endif //IMAGE_COLLECTION_H
