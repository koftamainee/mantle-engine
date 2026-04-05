#include "../vulkan/vulkan_utils.h"

#include <fstream>
#include <ios>

#include "core/assert.h"

namespace mantle {
    std::vector<uint32_t> load_spv(std::string_view path) {
        std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
        check(file.is_open());

        long size = file.tellg();
        check(size % 4 == 0);

        file.seekg(0);
        std::vector<uint32_t> buf(size / 4);
        file.read(reinterpret_cast<char *>(buf.data()), size);

        return buf;
    }
}
