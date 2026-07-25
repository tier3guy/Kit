#pragma once

#include "tree.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace kit {

// One recorded path inside a park.
struct ParkEntry {
    bool is_dir = false;
    std::uintmax_t size = 0; // files only
    std::string path;        // relative to the repository root
};

// A single parked state: the working directory as it looked when parked.
struct Park {
    std::int64_t created_at = 0; // unix epoch seconds
    std::string message;
    std::vector<ParkEntry> entries;
};

// The list of parks, stored in .kit/parks.
//
// Ordering: parks_ is chronological, so the back of the array is the newest
// park. Pushes append and pops take from the back, which makes this a stack --
// exactly the shelf semantics of git stash. Callers that want git's numbering
// (park@{0} == most recent) should walk the array in reverse.
class ParkList {
  public:
    // Reads .kit/parks. A missing file is not an error: it means no parks yet.
    // Returns false only if the file exists but could not be read or parsed.
    bool load(const std::filesystem::path& kit_dir);
    bool save(const std::filesystem::path& kit_dir) const;

    void push(Park park);
    std::optional<Park> pop();

    const std::vector<Park>& all() const { return parks_; }
    std::size_t size() const { return parks_.size(); }
    bool empty() const { return parks_.empty(); }

  private:
    std::vector<Park> parks_;
};

// Flattens a snapshot tree into the entry list a Park stores. Paths are
// relative to `root`; the root node itself is not emitted.
std::vector<ParkEntry> flatten(const Node& root);

} // namespace kit
