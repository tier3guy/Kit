#pragma once

#include <filesystem>

namespace kit {

// True if the working directory or the staging area differs from HEAD.
// Commands that overwrite tracked files (switch, join, undo) check this
// first so they never silently clobber work the user hasn't saved or
// parked yet.
bool has_uncommitted_changes(const std::filesystem::path& kit_dir, const std::filesystem::path& root);

} // namespace kit
