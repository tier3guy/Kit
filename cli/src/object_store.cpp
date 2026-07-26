#include "object_store.h"

#include "hash.h"

#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace kit {

std::string object_type_name(ObjectType type) {
    switch (type) {
        case ObjectType::Blob:
            return "blob";
        case ObjectType::Tree:
            return "tree";
        case ObjectType::Commit:
            return "commit";
    }
    return "";
}

std::optional<ObjectType> object_type_from_name(const std::string& name) {
    if (name == "blob") return ObjectType::Blob;
    if (name == "tree") return ObjectType::Tree;
    if (name == "commit") return ObjectType::Commit;
    return std::nullopt;
}

fs::path object_path(const fs::path& kit_dir, const std::string& hash) {
    return kit_dir / "objects" / hash.substr(0, 2) / hash.substr(2);
}

std::string hash_for(ObjectType type, const std::string& payload) {
    std::string content = object_type_name(type) + " " + std::to_string(payload.size());
    content += '\0';
    content += payload;
    return sha1_hex(content);
}

std::string write_object(const fs::path& kit_dir, ObjectType type, const std::string& payload) {
    const std::string hash = hash_for(type, payload);
    const fs::path path = object_path(kit_dir, hash);

    // Content-addressed: if it's already on disk, the bytes are identical
    // by construction, so there's nothing to do.
    if (!fs::exists(path)) {
        std::string content = object_type_name(type) + " " + std::to_string(payload.size());
        content += '\0';
        content += payload;
        std::error_code ec;
        fs::create_directories(path.parent_path(), ec);
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
    }

    return hash;
}

bool read_object(const fs::path& kit_dir, const std::string& hash, ObjectType& type_out,
                  std::string& payload_out) {
    std::ifstream in(object_path(kit_dir, hash), std::ios::binary);
    if (!in) {
        return false;
    }

    std::ostringstream ss;
    ss << in.rdbuf();
    const std::string content = ss.str();

    const auto space = content.find(' ');
    const auto nul = content.find('\0');
    if (space == std::string::npos || nul == std::string::npos || nul < space) {
        return false;
    }

    const auto type = object_type_from_name(content.substr(0, space));
    if (!type) {
        return false;
    }

    type_out = *type;
    payload_out = content.substr(nul + 1);
    return true;
}

} // namespace kit
