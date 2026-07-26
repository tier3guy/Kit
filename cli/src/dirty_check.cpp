#include "dirty_check.h"

#include "commit.h"
#include "index.h"
#include "object_store.h"
#include "refs.h"
#include "tree_walk.h"
#include "working_scan.h"

namespace fs = std::filesystem;

namespace kit {

namespace {

std::string head_tree_hash(const fs::path& kit_dir) {
    auto head = resolve_head(kit_dir);
    if (!head) return "";
    ObjectType type;
    std::string payload;
    if (!read_object(kit_dir, *head, type, payload) || type != ObjectType::Commit) return "";
    const auto commit = parse_commit(payload);
    return commit ? commit->tree : "";
}

} // namespace

bool has_uncommitted_changes(const fs::path& kit_dir, const fs::path& root) {
    Index index;
    index.load(kit_dir); // a load failure just means we compare against nothing staged

    FlatTree staged;
    for (const auto& e : index.entries()) {
        staged[e.path] = FlatEntry{e.hash, e.size};
    }

    const FlatTree head_files = flatten_tree(kit_dir, head_tree_hash(kit_dir));
    const FlatTree working_files = scan_working_tree(root);

    if (staged != head_files) {
        return true;
    }
    if (working_files != staged) {
        return true;
    }
    return false;
}

} // namespace kit
