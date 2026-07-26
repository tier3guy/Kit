#include "command.h"
#include "commit.h"
#include "dirty_check.h"
#include "index.h"
#include "merge_state.h"
#include "object_store.h"
#include "refs.h"
#include "tree_walk.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace {

std::string tree_of_commit(const fs::path& kit_dir, const std::string& commit_hash) {
    kit::ObjectType type;
    std::string payload;
    if (!kit::read_object(kit_dir, commit_hash, type, payload) || type != kit::ObjectType::Commit) {
        return "";
    }
    const auto commit = kit::parse_commit(payload);
    return commit ? commit->tree : "";
}

// Removes files that were tracked in `old_tree` but don't exist in
// `new_tree`, writes every file `new_tree` has, then rebuilds the index so
// it matches the new HEAD exactly.
void checkout_tree(const fs::path& kit_dir, const fs::path& root, const std::string& old_tree,
                    const std::string& new_tree) {
    const kit::FlatTree old_files = kit::flatten_tree(kit_dir, old_tree);
    const kit::FlatTree new_files = kit::flatten_tree(kit_dir, new_tree);

    for (const auto& [path, entry] : old_files) {
        if (!new_files.count(path)) {
            std::error_code ec;
            fs::remove(root / path, ec);
        }
    }

    kit::restore_tree(kit_dir, new_tree, root);
    kit::prune_empty_dirs(root);

    kit::Index index;
    for (const auto& [path, entry] : new_files) {
        kit::IndexEntry ie;
        ie.hash = entry.hash;
        ie.size = entry.size;
        ie.path = path;
        index.stage(ie);
    }
    index.save(kit_dir);
}

} // namespace

int cmd_switch(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cerr << "kit switch: usage: kit switch <name>\n";
        return 1;
    }

    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit switch: not a kit repository (no .kit here)\n";
        return 1;
    }

    const std::string& name = args[0];

    const kit::HeadState state = kit::read_head_state(kit_dir);
    if (!state.detached && state.branch == name) {
        std::cout << "already on '" << name << "'\n";
        return 0;
    }

    if (!kit::branch_exists(kit_dir, name)) {
        std::cerr << "kit switch: no such branch '" << name << "' (try 'kit fork " << name
                  << "')\n";
        return 1;
    }

    if (kit::merge_in_progress(kit_dir)) {
        std::cerr << "kit switch: a join is paused with conflicts -- resolve it or 'kit join "
                     "--abort' first\n";
        return 1;
    }

    if (kit::has_uncommitted_changes(kit_dir, root)) {
        std::cerr << "kit switch: you have unsaved changes -- 'kit save' or 'kit park' them first\n";
        return 1;
    }

    const auto current_commit = kit::resolve_head(kit_dir);
    const std::string old_tree = current_commit ? tree_of_commit(kit_dir, *current_commit) : "";

    const auto target_commit = kit::read_branch(kit_dir, name);
    const std::string new_tree = target_commit ? tree_of_commit(kit_dir, *target_commit) : "";

    checkout_tree(kit_dir, root, old_tree, new_tree);
    kit::attach_head_to_branch(kit_dir, name);

    std::cout << "switched to '" << name << "'\n";
    return 0;
}
