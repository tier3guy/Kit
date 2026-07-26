#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>

namespace kit {

struct FlatEntry {
    std::string hash;
    std::uintmax_t size = 0;

    bool operator==(const FlatEntry& other) const {
        return hash == other.hash && size == other.size;
    }
};

// path (relative, '/'-separated) -> blob entry, for every file reachable
// from a tree object. Directories themselves aren't listed -- callers only
// ever care about the files.
using FlatTree = std::map<std::string, FlatEntry>;

// Recursively reads a tree object and every subtree it references, and
// returns every file in it as a flat path -> blob map. Empty map if
// `tree_hash` is empty (used to represent "no commit yet").
FlatTree flatten_tree(const std::filesystem::path& kit_dir, const std::string& tree_hash);

// Writes every blob reachable from `tree_hash` onto disk under `root`,
// creating directories as needed. Does not delete anything -- callers that
// need a clean checkout should diff against the previous tree first (see
// flatten_tree) and remove files that no longer exist in the new one.
void restore_tree(const std::filesystem::path& kit_dir, const std::string& tree_hash,
                   const std::filesystem::path& root);

// Removes any now-empty directories under `root` (but never `root` itself
// and never .kit). Used after a checkout deletes files.
void prune_empty_dirs(const std::filesystem::path& root);

} // namespace kit
