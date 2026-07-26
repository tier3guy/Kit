#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace kit {

// While a `kit join` is paused for the user to resolve conflicts, the
// other side's commit lives in .kit/MERGE_HEAD (so `kit save` knows to
// create a two-parent merge commit) and the intended message lives in
// .kit/MERGE_MSG.
bool merge_in_progress(const std::filesystem::path& kit_dir);
std::optional<std::string> read_merge_head(const std::filesystem::path& kit_dir);
std::optional<std::string> read_merge_msg(const std::filesystem::path& kit_dir);
void write_merge_state(const std::filesystem::path& kit_dir, const std::string& other_commit,
                        const std::string& message);
void clear_merge_state(const std::filesystem::path& kit_dir);

} // namespace kit
