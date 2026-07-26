#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace kit {

// kit's ref model, deliberately close to git's:
//   .kit/HEAD               -- either "ref: refs/heads/<branch>" (attached)
//                               or a raw 40-char commit hash (detached)
//   .kit/refs/heads/<name>  -- a branch: the commit hash it currently points at
//   .kit/refs/tags/<name>   -- a mark: a commit hash that never moves on its own

// Where HEAD currently points.
struct HeadState {
    bool detached = false;
    std::string branch;      // valid when !detached
    std::string commit_hash; // valid when detached; empty if repo has no commits yet
};

HeadState read_head_state(const std::filesystem::path& kit_dir);

// Resolves HEAD down to a concrete commit hash, following the branch ref if
// attached. Returns nullopt if there are no commits yet on that ref.
std::optional<std::string> resolve_head(const std::filesystem::path& kit_dir);

void attach_head_to_branch(const std::filesystem::path& kit_dir, const std::string& branch);
void detach_head_to_commit(const std::filesystem::path& kit_dir, const std::string& commit_hash);

// Branch refs (refs/heads/<name>).
bool branch_exists(const std::filesystem::path& kit_dir, const std::string& name);
std::optional<std::string> read_branch(const std::filesystem::path& kit_dir, const std::string& name);
void write_branch(const std::filesystem::path& kit_dir, const std::string& name,
                   const std::string& commit_hash);
std::vector<std::string> list_branches(const std::filesystem::path& kit_dir);

// Tag refs (refs/tags/<name>) -- written once, never moved by kit itself.
bool tag_exists(const std::filesystem::path& kit_dir, const std::string& name);
std::optional<std::string> read_tag(const std::filesystem::path& kit_dir, const std::string& name);
void write_tag(const std::filesystem::path& kit_dir, const std::string& name,
               const std::string& commit_hash);
std::vector<std::string> list_tags(const std::filesystem::path& kit_dir);

// Resolves a user-supplied ref: a branch name, a tag name, the literal
// "HEAD", or a commit hash (full or a unique prefix). Returns nullopt if it
// doesn't resolve to anything, or resolves ambiguously.
std::optional<std::string> resolve_ref(const std::filesystem::path& kit_dir, const std::string& ref);

} // namespace kit
