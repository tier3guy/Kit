#include "command.h"
#include "commit.h"
#include "index.h"
#include "merge_state.h"
#include "object_store.h"
#include "refs.h"
#include "tree_walk.h"
#include "working_scan.h"

#include <filesystem>
#include <iostream>
#include <map>

namespace fs = std::filesystem;

namespace {

std::string head_tree(const fs::path& kit_dir) {
    auto head = kit::resolve_head(kit_dir);
    if (!head) {
        return "";
    }
    kit::ObjectType type;
    std::string payload;
    if (!kit::read_object(kit_dir, *head, type, payload) || type != kit::ObjectType::Commit) {
        return "";
    }
    const auto commit = kit::parse_commit(payload);
    return commit ? commit->tree : "";
}

kit::FlatTree index_as_flat(const kit::Index& index) {
    kit::FlatTree out;
    for (const auto& e : index.entries()) {
        out[e.path] = kit::FlatEntry{e.hash, e.size};
    }
    return out;
}

} // namespace

int cmd_check(const std::vector<std::string>& args) {
    if (!args.empty()) {
        std::cerr << "kit check: takes no arguments\n";
        return 1;
    }

    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit check: not a kit repository (no .kit here)\n";
        return 1;
    }

    const kit::HeadState state = kit::read_head_state(kit_dir);
    if (state.detached) {
        std::cout << "on detached HEAD at " << state.commit_hash.substr(0, 7) << "\n\n";
    } else {
        std::cout << "on " << state.branch << "\n\n";
    }

    if (kit::merge_in_progress(kit_dir)) {
        std::cout << "a join is paused with conflicts -- resolve them, 'kit stage' each file, "
                     "then 'kit save' (or 'kit join --abort')\n\n";
    }

    kit::Index index;
    if (!index.load(kit_dir)) {
        std::cerr << "kit check: could not read " << (kit_dir / "index").string() << "\n";
        return 1;
    }

    const kit::FlatTree head_files = kit::flatten_tree(kit_dir, head_tree(kit_dir));
    const kit::FlatTree staged_files = index_as_flat(index);
    const kit::FlatTree working_files = kit::scan_working_tree(root);

    // Staged changes: index vs the last save.
    bool any_staged = false;
    std::vector<std::string> staged_lines;
    {
        std::map<std::string, char> status; // path -> 'A'/'M'/'D'
        for (const auto& [path, entry] : staged_files) {
            const auto it = head_files.find(path);
            if (it == head_files.end()) {
                status[path] = 'A';
            } else if (it->second.hash != entry.hash) {
                status[path] = 'M';
            }
        }
        for (const auto& [path, entry] : head_files) {
            if (!staged_files.count(path)) {
                status[path] = 'D';
            }
        }
        for (const auto& [path, mark] : status) {
            any_staged = true;
            const char* label = mark == 'A' ? "new file:" : mark == 'M' ? "modified:" : "deleted: ";
            staged_lines.push_back(std::string("  ") + label + " " + path);
        }
    }

    // Unstaged changes: working directory vs the index.
    bool any_unstaged = false;
    std::vector<std::string> modified_lines;
    std::vector<std::string> deleted_lines;
    std::vector<std::string> untracked_lines;
    {
        for (const auto& [path, entry] : staged_files) {
            const auto it = working_files.find(path);
            if (it == working_files.end()) {
                deleted_lines.push_back("  deleted:  " + path);
            } else if (it->second.hash != entry.hash) {
                modified_lines.push_back("  modified: " + path);
            }
        }
        for (const auto& [path, entry] : working_files) {
            if (!staged_files.count(path)) {
                untracked_lines.push_back("  " + path);
            }
        }
        any_unstaged = !modified_lines.empty() || !deleted_lines.empty();
    }

    if (any_staged) {
        std::cout << "staged for the next save:\n";
        for (const auto& line : staged_lines) std::cout << line << "\n";
        std::cout << "\n";
    }

    if (any_unstaged) {
        std::cout << "changed but not staged:\n";
        for (const auto& line : modified_lines) std::cout << line << "\n";
        for (const auto& line : deleted_lines) std::cout << line << "\n";
        std::cout << "\n";
    }

    if (!untracked_lines.empty()) {
        std::cout << "untracked:\n";
        for (const auto& line : untracked_lines) std::cout << line << "\n";
        std::cout << "\n";
    }

    if (!any_staged && !any_unstaged && untracked_lines.empty()) {
        std::cout << "nothing to save, working directory clean\n";
    }

    return 0;
}
