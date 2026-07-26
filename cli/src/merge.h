#pragma once

#include "tree_walk.h"

#include <filesystem>
#include <string>
#include <vector>

namespace kit {

// A path that changed differently on both sides of a merge.
struct MergeConflict {
    std::string path;
    // True for a type mismatch (file vs. directory on the two sides) --
    // there's no sensible text marker for that, so this always forces the
    // whole merge to abort. False means an ordinary content conflict: two
    // sides edited the same file differently, which can be marked inline
    // and left for the user to resolve.
    bool structural = false;
    std::string base_hash; // empty if the path didn't exist at the merge base
    std::string a_hash;    // empty if deleted on side A
    std::string b_hash;    // empty if deleted on side B
};

struct MergeOutcome {
    // Every path the merge could resolve automatically (unchanged, or
    // changed on only one side).
    FlatTree resolved;
    std::vector<MergeConflict> conflicts;
};

// Performs a 3-way merge of two trees against their common ancestor tree.
// Doesn't write anything to the object store or disk -- purely computes
// the outcome, so the caller can decide how to handle conflicts (kit join
// writes conflict markers for content conflicts, but aborts outright if any
// conflict is structural).
MergeOutcome merge_trees(const std::filesystem::path& kit_dir, const std::string& base_hash,
                          const std::string& a_hash, const std::string& b_hash);

} // namespace kit
