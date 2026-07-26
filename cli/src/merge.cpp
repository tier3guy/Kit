#include "merge.h"

#include "object_store.h"
#include "tree_object.h"

#include <map>

namespace fs = std::filesystem;

namespace kit {

namespace {

std::map<std::string, TreeEntry> load_tree(const fs::path& kit_dir, const std::string& hash) {
    std::map<std::string, TreeEntry> entries;
    if (hash.empty()) {
        return entries;
    }
    ObjectType type;
    std::string payload;
    if (!read_object(kit_dir, hash, type, payload) || type != ObjectType::Tree) {
        return entries;
    }
    for (TreeEntry& e : parse_tree(payload)) {
        entries[e.name] = std::move(e);
    }
    return entries;
}

std::string join_path(const std::string& prefix, const std::string& name) {
    return prefix.empty() ? name : prefix + "/" + name;
}

std::uintmax_t blob_size(const fs::path& kit_dir, const std::string& hash) {
    if (hash.empty()) return 0;
    ObjectType type;
    std::string payload;
    if (read_object(kit_dir, hash, type, payload)) return payload.size();
    return 0;
}

void merge_recursive(const fs::path& kit_dir, const std::string& base_hash,
                      const std::string& a_hash, const std::string& b_hash,
                      const std::string& prefix, MergeOutcome& out) {
    const auto base_entries = load_tree(kit_dir, base_hash);
    const auto a_entries = load_tree(kit_dir, a_hash);
    const auto b_entries = load_tree(kit_dir, b_hash);

    std::vector<std::string> names;
    for (const auto& [name, e] : base_entries) names.push_back(name);
    for (const auto& [name, e] : a_entries)
        if (!base_entries.count(name)) names.push_back(name);
    for (const auto& [name, e] : b_entries)
        if (!base_entries.count(name) && !a_entries.count(name)) names.push_back(name);

    for (const std::string& name : names) {
        const auto base_it = base_entries.find(name);
        const auto a_it = a_entries.find(name);
        const auto b_it = b_entries.find(name);

        const bool in_base = base_it != base_entries.end();
        const bool in_a = a_it != a_entries.end();
        const bool in_b = b_it != b_entries.end();
        const std::string path = join_path(prefix, name);

        std::optional<std::string> common_type;
        bool type_conflict = false;
        if (in_base) common_type = base_it->second.type;
        if (in_a) {
            if (common_type && *common_type != a_it->second.type) type_conflict = true;
            common_type = a_it->second.type;
        }
        if (in_b) {
            if (common_type && *common_type != b_it->second.type) type_conflict = true;
            common_type = b_it->second.type;
        }

        if (type_conflict) {
            out.conflicts.push_back(MergeConflict{path, true, "", "", ""});
            continue;
        }

        if (common_type && *common_type == "tree") {
            merge_recursive(kit_dir, in_base ? base_it->second.hash : "",
                             in_a ? a_it->second.hash : "", in_b ? b_it->second.hash : "", path,
                             out);
            continue;
        }

        const std::string base_hash_v = in_base ? base_it->second.hash : "";
        const std::string a_hash_v = in_a ? a_it->second.hash : "";
        const std::string b_hash_v = in_b ? b_it->second.hash : "";

        if (a_hash_v == b_hash_v) {
            if (in_a) out.resolved[path] = FlatEntry{a_hash_v, blob_size(kit_dir, a_hash_v)};
            // absent on both, or identical on both -- nothing to add otherwise
            continue;
        }
        if (a_hash_v == base_hash_v) {
            // A unchanged from base, B is the real change (add/modify/delete).
            if (in_b) out.resolved[path] = FlatEntry{b_hash_v, blob_size(kit_dir, b_hash_v)};
            continue;
        }
        if (b_hash_v == base_hash_v) {
            // B unchanged from base, A is the real change.
            if (in_a) out.resolved[path] = FlatEntry{a_hash_v, blob_size(kit_dir, a_hash_v)};
            continue;
        }

        // Both sides changed this path differently -- content conflict.
        out.conflicts.push_back(MergeConflict{path, false, base_hash_v, a_hash_v, b_hash_v});
    }
}

} // namespace

MergeOutcome merge_trees(const fs::path& kit_dir, const std::string& base_hash,
                          const std::string& a_hash, const std::string& b_hash) {
    MergeOutcome out;
    merge_recursive(kit_dir, base_hash, a_hash, b_hash, "", out);
    return out;
}

} // namespace kit
