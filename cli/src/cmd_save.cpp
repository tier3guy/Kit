#include "command.h"
#include "commit.h"
#include "index.h"
#include "merge_state.h"
#include "object_store.h"
#include "refs.h"
#include "tree_object.h"

#include <ctime>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace {

// Refuses to save a file that still contains unresolved conflict markers
// left by a paused `kit join`. Catches the common slip of forgetting to
// clean one up before staging it.
std::vector<std::string> files_with_markers(const fs::path& kit_dir, const kit::Index& index) {
    std::vector<std::string> offenders;
    for (const auto& e : index.entries()) {
        kit::ObjectType type;
        std::string payload;
        if (kit::read_object(kit_dir, e.hash, type, payload) && type == kit::ObjectType::Blob) {
            if (payload.find("<<<<<<< ") != std::string::npos ||
                payload.find(">>>>>>> ") != std::string::npos) {
                offenders.push_back(e.path);
            }
        }
    }
    return offenders;
}

} // namespace

int cmd_save(const std::vector<std::string>& args) {
    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit save: not a kit repository (no .kit here)\n";
        return 1;
    }

    std::vector<std::string> rest = args;
    if (!rest.empty() && rest[0] == "-m") {
        rest.erase(rest.begin());
    }

    std::string message;
    for (std::size_t i = 0; i < rest.size(); ++i) {
        if (i > 0) {
            message += " ";
        }
        message += rest[i];
    }

    const auto merge_head = kit::read_merge_head(kit_dir);

    if (message.empty()) {
        // Finishing a paused join without -m falls back to the message it
        // was started with, same as git does for a merge commit.
        if (auto merge_msg = merge_head ? kit::read_merge_msg(kit_dir) : std::nullopt) {
            message = *merge_msg;
        }
    }
    if (message.empty()) {
        std::cerr << "kit save: missing message (use 'kit save -m \"message\"')\n";
        return 1;
    }

    kit::Index index;
    if (!index.load(kit_dir)) {
        std::cerr << "kit save: could not read " << (kit_dir / "index").string() << "\n";
        return 1;
    }
    if (index.empty()) {
        std::cerr << "kit save: nothing staged (use 'kit stage' first)\n";
        return 1;
    }

    if (merge_head) {
        const auto offenders = files_with_markers(kit_dir, index);
        if (!offenders.empty()) {
            std::cerr << "kit save: these files still have unresolved conflict markers:\n";
            for (const auto& path : offenders) std::cerr << "  " << path << "\n";
            std::cerr << "clean them up, 'kit stage' them again, then 'kit save'\n";
            return 1;
        }
    }

    const std::string tree_hash = kit::write_tree_from_index(kit_dir, index.entries());

    // Refuse a no-op save -- same check git does (a commit whose tree is
    // identical to HEAD's is presumably a mistake, not a merge commit,
    // since merges are handled separately above and always have their own
    // two-parent identity even with a matching tree).
    if (!merge_head) {
        if (auto parent = kit::resolve_head(kit_dir)) {
            kit::ObjectType parent_type;
            std::string parent_payload;
            if (kit::read_object(kit_dir, *parent, parent_type, parent_payload) &&
                parent_type == kit::ObjectType::Commit) {
                if (const auto parent_commit = kit::parse_commit(parent_payload)) {
                    if (parent_commit->tree == tree_hash) {
                        std::cerr << "kit save: nothing changed since the last save\n";
                        return 1;
                    }
                }
            }
        }
    }

    kit::CommitData commit;
    commit.tree = tree_hash;
    if (auto parent = kit::resolve_head(kit_dir)) {
        commit.parents.push_back(*parent);
    }
    if (merge_head) {
        // Completing a paused join: this save becomes a merge commit with
        // both branch tips as parents.
        commit.parents.push_back(*merge_head);
    }
    commit.time = static_cast<std::int64_t>(std::time(nullptr));
    commit.message = message;

    const std::string commit_hash =
        kit::write_object(kit_dir, kit::ObjectType::Commit, kit::serialize_commit(commit));

    const kit::HeadState state = kit::read_head_state(kit_dir);
    if (state.detached) {
        kit::detach_head_to_commit(kit_dir, commit_hash);
    } else {
        kit::write_branch(kit_dir, state.branch, commit_hash);
    }

    if (merge_head) {
        kit::clear_merge_state(kit_dir);
    }

    std::cout << "saved " << commit_hash.substr(0, 7) << ": " << message << "\n";
    return 0;
}
