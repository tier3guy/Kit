#include "ancestry.h"
#include "command.h"
#include "commit.h"
#include "dirty_check.h"
#include "index.h"
#include "merge.h"
#include "merge_state.h"
#include "object_store.h"
#include "refs.h"
#include "tree_object.h"
#include "tree_walk.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
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

std::string blob_content(const fs::path& kit_dir, const std::string& hash) {
    if (hash.empty()) return "";
    kit::ObjectType type;
    std::string payload;
    if (read_object(kit_dir, hash, type, payload) && type == kit::ObjectType::Blob) {
        return payload;
    }
    return "";
}

// Same as blob_content but with any trailing newline stripped, so building
// a conflict marker block never doubles up blank lines around "=======".
std::string blob_content_trimmed(const fs::path& kit_dir, const std::string& hash) {
    std::string payload = blob_content(kit_dir, hash);
    while (!payload.empty() && payload.back() == '\n') payload.pop_back();
    return payload;
}

int abort_join(const fs::path& kit_dir, const fs::path& root) {
    const auto other_commit = kit::read_merge_head(kit_dir);
    if (!other_commit) {
        std::cerr << "kit join: no join in progress\n";
        return 1;
    }

    const kit::HeadState state = kit::read_head_state(kit_dir);
    const auto current_commit = state.detached ? std::make_optional(state.commit_hash)
                                                : kit::read_branch(kit_dir, state.branch);
    if (!current_commit) {
        std::cerr << "kit join: could not resolve the current save\n";
        return 1;
    }

    // Recompute exactly the same merge to find every path it touched, so
    // abort only reverts those -- it never guesses at unrelated files.
    const auto base_commit = kit::merge_base(kit_dir, *current_commit, *other_commit);
    const std::string base_tree = base_commit ? tree_of_commit(kit_dir, *base_commit) : "";
    const std::string current_tree = tree_of_commit(kit_dir, *current_commit);
    const std::string other_tree = tree_of_commit(kit_dir, *other_commit);
    const kit::MergeOutcome outcome = kit::merge_trees(kit_dir, base_tree, current_tree, other_tree);

    const kit::FlatTree original = kit::flatten_tree(kit_dir, current_tree);

    auto revert_path = [&](const std::string& path) {
        const auto it = original.find(path);
        std::error_code ec;
        if (it == original.end()) {
            fs::remove(root / path, ec); // didn't exist before the merge attempt
            return;
        }
        kit::ObjectType type;
        std::string content;
        if (kit::read_object(kit_dir, it->second.hash, type, content) && type == kit::ObjectType::Blob) {
            fs::path abs = root / path;
            fs::create_directories(abs.parent_path(), ec);
            std::ofstream out(abs, std::ios::binary | std::ios::trunc);
            out.write(content.data(), static_cast<std::streamsize>(content.size()));
        }
    };

    for (const auto& [path, entry] : outcome.resolved) revert_path(path);
    for (const auto& c : outcome.conflicts) revert_path(c.path);
    kit::prune_empty_dirs(root);

    kit::Index index;
    for (const auto& [path, entry] : original) {
        kit::IndexEntry ie;
        ie.hash = entry.hash;
        ie.size = entry.size;
        ie.path = path;
        index.stage(ie);
    }
    index.save(kit_dir);

    kit::clear_merge_state(kit_dir);
    std::cout << "aborted join -- back to " << current_commit->substr(0, 7) << "\n";
    return 0;
}

} // namespace

int cmd_join(const std::vector<std::string>& args) {
    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit join: not a kit repository (no .kit here)\n";
        return 1;
    }

    if (args.size() == 1 && args[0] == "--abort") {
        return abort_join(kit_dir, root);
    }

    if (args.size() != 1) {
        std::cerr << "kit join: usage: kit join <branch> | kit join --abort\n";
        return 1;
    }

    if (kit::merge_in_progress(kit_dir)) {
        std::cerr << "kit join: a join is already in progress -- resolve the conflicts and "
                     "'kit save', or run 'kit join --abort'\n";
        return 1;
    }

    const kit::HeadState state = kit::read_head_state(kit_dir);
    if (state.detached) {
        std::cerr << "kit join: HEAD is detached -- switch to a branch first\n";
        return 1;
    }

    const std::string& other_name = args[0];
    const auto other_commit = kit::read_branch(kit_dir, other_name);
    if (!other_commit) {
        std::cerr << "kit join: no such branch '" << other_name << "'\n";
        return 1;
    }

    const auto current_commit = kit::read_branch(kit_dir, state.branch);

    if (!current_commit) {
        if (kit::has_uncommitted_changes(kit_dir, root)) {
            std::cerr << "kit join: you have unsaved changes -- 'kit save' or 'kit park' them first\n";
            return 1;
        }
        checkout_tree(kit_dir, root, "", tree_of_commit(kit_dir, *other_commit));
        kit::write_branch(kit_dir, state.branch, *other_commit);
        std::cout << "fast-forwarded '" << state.branch << "' to " << other_commit->substr(0, 7)
                  << "\n";
        return 0;
    }

    if (*current_commit == *other_commit ||
        kit::is_ancestor(kit_dir, *other_commit, *current_commit)) {
        std::cout << "already up to date\n";
        return 0;
    }

    if (kit::has_uncommitted_changes(kit_dir, root)) {
        std::cerr << "kit join: you have unsaved changes -- 'kit save' or 'kit park' them first\n";
        return 1;
    }

    if (kit::is_ancestor(kit_dir, *current_commit, *other_commit)) {
        checkout_tree(kit_dir, root, tree_of_commit(kit_dir, *current_commit),
                      tree_of_commit(kit_dir, *other_commit));
        kit::write_branch(kit_dir, state.branch, *other_commit);
        std::cout << "fast-forwarded '" << state.branch << "' to " << other_commit->substr(0, 7)
                  << "\n";
        return 0;
    }

    // Real merge: 3-way, using the nearest common ancestor as the base.
    const auto base_commit = kit::merge_base(kit_dir, *current_commit, *other_commit);
    const std::string base_tree = base_commit ? tree_of_commit(kit_dir, *base_commit) : "";
    const std::string current_tree = tree_of_commit(kit_dir, *current_commit);
    const std::string other_tree = tree_of_commit(kit_dir, *other_commit);

    const kit::MergeOutcome outcome = kit::merge_trees(kit_dir, base_tree, current_tree, other_tree);

    const bool has_structural =
        std::any_of(outcome.conflicts.begin(), outcome.conflicts.end(),
                    [](const kit::MergeConflict& c) { return c.structural; });

    if (has_structural) {
        std::cerr << "kit join: can't merge automatically -- these paths changed incompatibly "
                     "(file vs. directory) on both sides:\n";
        for (const auto& c : outcome.conflicts) {
            if (c.structural) std::cerr << "  " << c.path << "\n";
        }
        std::cerr << "nothing was changed\n";
        return 1;
    }

    if (!outcome.conflicts.empty()) {
        // Content conflicts only: apply everything that resolved cleanly,
        // write conflict markers for the rest, and pause the merge for the
        // user to sort out -- same shape as a real git conflicted merge.
        const kit::FlatTree old_files = kit::flatten_tree(kit_dir, current_tree);

        for (const auto& [path, entry] : old_files) {
            if (!outcome.resolved.count(path)) {
                bool still_conflicted = false;
                for (const auto& c : outcome.conflicts) {
                    if (c.path == path) still_conflicted = true;
                }
                if (!still_conflicted) {
                    std::error_code ec;
                    fs::remove(root / path, ec);
                }
            }
        }

        kit::Index index;
        index.load(kit_dir);
        for (const auto& [path, entry] : outcome.resolved) {
            fs::path abs = root / path;
            std::error_code ec;
            fs::create_directories(abs.parent_path(), ec);
            const std::string content = blob_content(kit_dir, entry.hash);
            std::ofstream out(abs, std::ios::binary | std::ios::trunc);
            out.write(content.data(), static_cast<std::streamsize>(content.size()));

            kit::IndexEntry ie;
            ie.hash = entry.hash;
            ie.size = entry.size;
            ie.path = path;
            index.stage(ie);
        }

        for (const auto& c : outcome.conflicts) {
            fs::path abs = root / c.path;
            std::error_code ec;
            fs::create_directories(abs.parent_path(), ec);
            std::ofstream out(abs, std::ios::trunc);
            out << "<<<<<<< " << state.branch << "\n";
            const std::string a_text = blob_content_trimmed(kit_dir, c.a_hash);
            if (!a_text.empty()) out << a_text << "\n";
            out << "=======\n";
            const std::string b_text = blob_content_trimmed(kit_dir, c.b_hash);
            if (!b_text.empty()) out << b_text << "\n";
            out << ">>>>>>> " << other_name << "\n";
        }
        index.save(kit_dir);

        kit::write_merge_state(kit_dir, *other_commit,
                                "join '" + other_name + "' into '" + state.branch + "'");

        std::cerr << "kit join: " << outcome.conflicts.size()
                  << " file(s) need manual resolution:\n";
        for (const auto& c : outcome.conflicts) {
            std::cerr << "  " << c.path << "\n";
        }
        std::cerr << "edit them, remove the <<<<<<< ======= >>>>>>> markers, then 'kit stage "
                     "<file>' each and 'kit save' to finish -- or 'kit join --abort' to cancel\n";
        return 1;
    }

    // No conflicts at all: build the merged tree and commit straight away.
    std::vector<kit::IndexEntry> resolved_entries;
    for (const auto& [path, entry] : outcome.resolved) {
        kit::IndexEntry ie;
        ie.hash = entry.hash;
        ie.size = entry.size;
        ie.path = path;
        resolved_entries.push_back(std::move(ie));
    }
    const std::string merged_tree = kit::write_tree_from_index(kit_dir, resolved_entries);

    kit::CommitData merge_commit;
    merge_commit.tree = merged_tree;
    merge_commit.parents = {*current_commit, *other_commit};
    merge_commit.time = static_cast<std::int64_t>(std::time(nullptr));
    merge_commit.message = "join '" + other_name + "' into '" + state.branch + "'";

    const std::string merge_hash =
        kit::write_object(kit_dir, kit::ObjectType::Commit, kit::serialize_commit(merge_commit));

    checkout_tree(kit_dir, root, current_tree, merged_tree);
    kit::write_branch(kit_dir, state.branch, merge_hash);

    std::cout << "joined '" << other_name << "' into '" << state.branch << "' as "
              << merge_hash.substr(0, 7) << "\n";
    return 0;
}
