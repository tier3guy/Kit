#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace kit {

// One staged file. `hash` points at a blob already written to the object
// store by the time it lands here -- the index never holds file content
// itself, only a reference to it.
struct IndexEntry {
    std::string mode = "100644"; // kit only tracks regular files
    std::string hash;            // blob hash
    std::uintmax_t size = 0;
    std::string path; // relative to the repository root, '/'-separated
};

// The staging area, persisted at .kit/index. `kit stage` writes to it,
// `kit save` reads it to build the tree for a commit.
class Index {
  public:
    bool load(const std::filesystem::path& kit_dir);
    bool save(const std::filesystem::path& kit_dir) const;

    // Adds a new entry or replaces the existing one for the same path.
    void stage(IndexEntry entry);

    const std::vector<IndexEntry>& entries() const { return entries_; }
    bool empty() const { return entries_.empty(); }

  private:
    std::vector<IndexEntry> entries_;
};

} // namespace kit
