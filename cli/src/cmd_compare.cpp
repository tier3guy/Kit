#include "command.h"
#include "commit.h"
#include "diff.h"
#include "index.h"
#include "object_store.h"
#include "refs.h"
#include "tree_walk.h"
#include "working_scan.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

namespace fs = std::filesystem;

namespace {

// One side of a comparison: either the live working directory, or a fixed
// tree (from the index or a commit).
struct Snapshot {
    kit::FlatTree files;
    bool is_working = false;
    std::string label;
};

std::string read_disk_file(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::optional<std::string> content_at(const fs::path& kit_dir, const fs::path& root,
                                       const Snapshot& snap, const std::string& path) {
    const auto it = snap.files.find(path);
    if (it == snap.files.end()) {
        return std::nullopt;
    }
    if (snap.is_working) {
        return read_disk_file(root / path);
    }
    kit::ObjectType type;
    std::string payload;
    if (!kit::read_object(kit_dir, it->second.hash, type, payload) || type != kit::ObjectType::Blob) {
        return std::nullopt;
    }
    return payload;
}

std::optional<std::string> resolve_tree_of(const fs::path& kit_dir, const std::string& ref) {
    const auto commit_hash = kit::resolve_ref(kit_dir, ref);
    if (!commit_hash) {
        return std::nullopt;
    }
    kit::ObjectType type;
    std::string payload;
    if (!kit::read_object(kit_dir, *commit_hash, type, payload) || type != kit::ObjectType::Commit) {
        return std::nullopt;
    }
    const auto commit = kit::parse_commit(payload);
    return commit ? std::make_optional(commit->tree) : std::nullopt;
}

void print_file_diff(const std::string& path, const std::optional<std::string>& old_content,
                      const std::optional<std::string>& new_content) {
    std::cout << "--- " << (old_content ? path : "/dev/null") << "\n";
    std::cout << "+++ " << (new_content ? path : "/dev/null") << "\n";

    const auto old_lines = kit::split_lines(old_content.value_or(""));
    const auto new_lines = kit::split_lines(new_content.value_or(""));
    for (const auto& line : kit::diff_lines(old_lines, new_lines)) {
        switch (line.op) {
            case kit::DiffOp::Equal:
                std::cout << "  " << line.text << "\n";
                break;
            case kit::DiffOp::Remove:
                std::cout << "- " << line.text << "\n";
                break;
            case kit::DiffOp::Add:
                std::cout << "+ " << line.text << "\n";
                break;
        }
    }
    std::cout << "\n";
}

} // namespace

int cmd_compare(const std::vector<std::string>& args) {
    if (args.size() > 2) {
        std::cerr << "kit compare: usage: kit compare [<ref>] [<ref>]\n";
        return 1;
    }

    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit compare: not a kit repository (no .kit here)\n";
        return 1;
    }

    Snapshot a, b;

    if (args.empty()) {
        // Unstaged changes: working directory vs what's staged.
        kit::Index index;
        if (!index.load(kit_dir)) {
            std::cerr << "kit compare: could not read " << (kit_dir / "index").string() << "\n";
            return 1;
        }
        for (const auto& e : index.entries()) {
            a.files[e.path] = kit::FlatEntry{e.hash, e.size};
        }
        a.label = "staged";
        b.files = kit::scan_working_tree(root);
        b.is_working = true;
        b.label = "working directory";
    } else if (args.size() == 1) {
        const auto tree = resolve_tree_of(kit_dir, args[0]);
        if (!tree) {
            std::cerr << "kit compare: " << args[0] << ": no such ref or commit\n";
            return 1;
        }
        a.files = kit::flatten_tree(kit_dir, *tree);
        a.label = args[0];
        b.files = kit::scan_working_tree(root);
        b.is_working = true;
        b.label = "working directory";
    } else {
        const auto tree_a = resolve_tree_of(kit_dir, args[0]);
        const auto tree_b = resolve_tree_of(kit_dir, args[1]);
        if (!tree_a) {
            std::cerr << "kit compare: " << args[0] << ": no such ref or commit\n";
            return 1;
        }
        if (!tree_b) {
            std::cerr << "kit compare: " << args[1] << ": no such ref or commit\n";
            return 1;
        }
        a.files = kit::flatten_tree(kit_dir, *tree_a);
        a.label = args[0];
        b.files = kit::flatten_tree(kit_dir, *tree_b);
        b.label = args[1];
    }

    std::set<std::string> paths;
    for (const auto& [path, entry] : a.files) paths.insert(path);
    for (const auto& [path, entry] : b.files) paths.insert(path);

    bool any = false;
    for (const std::string& path : paths) {
        const auto a_it = a.files.find(path);
        const auto b_it = b.files.find(path);
        const bool in_a = a_it != a.files.end();
        const bool in_b = b_it != b.files.end();

        if (in_a && in_b && a_it->second.hash == b_it->second.hash) {
            continue; // unchanged
        }

        any = true;
        std::cout << "diff  " << a.label << "/" << path << "  " << b.label << "/" << path << "\n";
        print_file_diff(path, content_at(kit_dir, root, a, path), content_at(kit_dir, root, b, path));
    }

    if (!any) {
        std::cout << "no differences\n";
    }

    return 0;
}
