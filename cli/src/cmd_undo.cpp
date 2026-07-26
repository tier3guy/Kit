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

int cmd_undo(const std::vector<std::string>& args) {
    if (!args.empty()) {
        std::cerr << "kit undo: takes no arguments\n";
        return 1;
    }

    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit undo: not a kit repository (no .kit here)\n";
        return 1;
    }

    if (kit::merge_in_progress(kit_dir)) {
        std::cerr << "kit undo: a join is paused with conflicts -- resolve it or 'kit join "
                     "--abort' first\n";
        return 1;
    }

    const kit::HeadState state = kit::read_head_state(kit_dir);
    const auto current = state.detached ? (state.commit_hash.empty()
                                                ? std::nullopt
                                                : std::make_optional(state.commit_hash))
                                         : kit::read_branch(kit_dir, state.branch);
    if (!current) {
        std::cerr << "kit undo: nothing to undo -- no saves yet\n";
        return 1;
    }

    kit::ObjectType type;
    std::string payload;
    if (!kit::read_object(kit_dir, *current, type, payload) || type != kit::ObjectType::Commit) {
        std::cerr << "kit undo: corrupt commit " << *current << "\n";
        return 1;
    }
    const auto commit = kit::parse_commit(payload);
    if (!commit || commit->parents.empty()) {
        std::cerr << "kit undo: already at the first save -- nothing before it\n";
        return 1;
    }

    if (kit::has_uncommitted_changes(kit_dir, root)) {
        std::cerr << "kit undo: you have unsaved changes -- 'kit save' or 'kit park' them first\n";
        return 1;
    }

    const std::string parent_hash = commit->parents.front();
    kit::ObjectType parent_type;
    std::string parent_payload;
    std::string parent_tree;
    if (kit::read_object(kit_dir, parent_hash, parent_type, parent_payload) &&
        parent_type == kit::ObjectType::Commit) {
        if (const auto parent_commit = kit::parse_commit(parent_payload)) {
            parent_tree = parent_commit->tree;
        }
    }

    const kit::FlatTree old_files = kit::flatten_tree(kit_dir, commit->tree);
    const kit::FlatTree new_files = kit::flatten_tree(kit_dir, parent_tree);
    for (const auto& [path, entry] : old_files) {
        if (!new_files.count(path)) {
            std::error_code ec;
            fs::remove(root / path, ec);
        }
    }
    kit::restore_tree(kit_dir, parent_tree, root);
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

    if (state.detached) {
        kit::detach_head_to_commit(kit_dir, parent_hash);
    } else {
        kit::write_branch(kit_dir, state.branch, parent_hash);
    }

    std::cout << "undid " << current->substr(0, 7) << ": " << commit->message << "\n";
    std::cout << "now at " << parent_hash.substr(0, 7) << "\n";
    return 0;
}
