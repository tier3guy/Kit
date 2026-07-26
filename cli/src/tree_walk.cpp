#include "tree_walk.h"

#include "object_store.h"
#include "tree_object.h"

#include <fstream>

namespace fs = std::filesystem;

namespace kit {

namespace {

void flatten_into(const fs::path& kit_dir, const std::string& tree_hash, const std::string& prefix,
                   FlatTree& out) {
    ObjectType type;
    std::string payload;
    if (!read_object(kit_dir, tree_hash, type, payload) || type != ObjectType::Tree) {
        return; // corrupt or missing -- treat as empty
    }

    for (const TreeEntry& entry : parse_tree(payload)) {
        const std::string path = prefix.empty() ? entry.name : prefix + "/" + entry.name;
        if (entry.type == "tree") {
            flatten_into(kit_dir, entry.hash, path, out);
        } else {
            ObjectType blob_type;
            std::string blob_payload;
            std::uintmax_t size = 0;
            if (read_object(kit_dir, entry.hash, blob_type, blob_payload)) {
                size = blob_payload.size();
            }
            out[path] = FlatEntry{entry.hash, size};
        }
    }
}

} // namespace

FlatTree flatten_tree(const fs::path& kit_dir, const std::string& tree_hash) {
    FlatTree out;
    if (!tree_hash.empty()) {
        flatten_into(kit_dir, tree_hash, "", out);
    }
    return out;
}

void restore_tree(const fs::path& kit_dir, const std::string& tree_hash, const fs::path& root) {
    for (const auto& [path, entry] : flatten_tree(kit_dir, tree_hash)) {
        ObjectType type;
        std::string content;
        if (!read_object(kit_dir, entry.hash, type, content) || type != ObjectType::Blob) {
            continue;
        }
        const fs::path abs_path = root / path;
        std::error_code ec;
        fs::create_directories(abs_path.parent_path(), ec);
        std::ofstream out(abs_path, std::ios::binary | std::ios::trunc);
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
    }
}

void prune_empty_dirs(const fs::path& root) {
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(root, ec)) {
        if (!entry.is_directory()) continue;
        if (entry.path().filename() == ".kit") continue;
        prune_empty_dirs(entry.path());
        std::error_code inner_ec;
        if (fs::is_empty(entry.path(), inner_ec) && !inner_ec) {
            fs::remove(entry.path(), inner_ec);
        }
    }
}

} // namespace kit
