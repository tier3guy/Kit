#pragma once

#include "index.h"

#include <filesystem>
#include <string>
#include <vector>

namespace kit {

// One line inside a serialized Tree object -- not to be confused with
// tree.h's Node, which is an in-memory scan of the *working directory*.
// A TreeEntry is a committed, content-addressed reference: it points at
// another Tree object (a directory) or a Blob object (a file) by hash.
struct TreeEntry {
    std::string mode; // "100644" file, "040000" directory
    std::string type; // "blob" or "tree"
    std::string hash;
    std::string name;
};

std::string serialize_tree(const std::vector<TreeEntry>& entries);
std::vector<TreeEntry> parse_tree(const std::string& payload);

// Builds the full (possibly nested) tree object graph implied by the
// staged index and writes every tree object to the store, bottom-up.
// Returns the hash of the root tree. `entries` must be non-empty.
std::string write_tree_from_index(const std::filesystem::path& kit_dir,
                                   const std::vector<IndexEntry>& entries);

} // namespace kit
