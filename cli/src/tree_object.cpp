#include "tree_object.h"

#include "object_store.h"

#include <algorithm>
#include <map>
#include <sstream>

namespace kit {

namespace {

// A directory being assembled from staged paths, before it's serialized
// into a Tree object. Kept as maps so both files and subdirs come out
// name-sorted automatically -- trees must be deterministic.
struct PendingDir {
    std::map<std::string, IndexEntry> files;
    std::map<std::string, PendingDir> subdirs;
};

void insert(PendingDir& root, const IndexEntry& entry) {
    PendingDir* cur = &root;
    std::string remaining = entry.path;
    std::size_t pos;
    while ((pos = remaining.find('/')) != std::string::npos) {
        cur = &cur->subdirs[remaining.substr(0, pos)];
        remaining = remaining.substr(pos + 1);
    }
    cur->files[remaining] = entry;
}

std::string write_dir(const std::filesystem::path& kit_dir, const PendingDir& dir) {
    std::vector<TreeEntry> entries;

    for (const auto& [name, subdir] : dir.subdirs) {
        entries.push_back(TreeEntry{"040000", "tree", write_dir(kit_dir, subdir), name});
    }
    for (const auto& [name, file] : dir.files) {
        entries.push_back(TreeEntry{file.mode, "blob", file.hash, name});
    }

    return write_object(kit_dir, ObjectType::Tree, serialize_tree(entries));
}

} // namespace

std::string serialize_tree(const std::vector<TreeEntry>& entries) {
    std::ostringstream out;
    for (const auto& e : entries) {
        out << e.mode << " " << e.type << " " << e.hash << "\t" << e.name << "\n";
    }
    return out.str();
}

std::vector<TreeEntry> parse_tree(const std::string& payload) {
    std::vector<TreeEntry> entries;
    std::istringstream in(payload);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream ls(line);
        TreeEntry e;
        ls >> e.mode >> e.type >> e.hash >> std::ws;
        std::getline(ls, e.name);
        entries.push_back(std::move(e));
    }
    return entries;
}

std::string write_tree_from_index(const std::filesystem::path& kit_dir,
                                   const std::vector<IndexEntry>& entries) {
    PendingDir root;
    for (const auto& e : entries) {
        insert(root, e);
    }
    return write_dir(kit_dir, root);
}

} // namespace kit
