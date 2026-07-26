#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace kit {

// Is `ancestor` reachable by walking parent links from `commit` (including
// commit == ancestor)? Used to detect fast-forwards.
bool is_ancestor(const std::filesystem::path& kit_dir, const std::string& ancestor,
                  const std::string& commit);

// Finds the best common ancestor of two commits (the merge base kit join
// needs for a proper 3-way tree merge). Returns nullopt if the two commits
// share no history at all.
std::optional<std::string> merge_base(const std::filesystem::path& kit_dir, const std::string& a,
                                       const std::string& b);

} // namespace kit
