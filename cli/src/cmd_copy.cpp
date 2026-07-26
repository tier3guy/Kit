#include "command.h"
#include "commit.h"
#include "index.h"
#include "object_store.h"
#include "refs.h"
#include "tree_walk.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace {

bool looks_like_url(const std::string& s) {
    return s.rfind("http://", 0) == 0 || s.rfind("https://", 0) == 0 || s.rfind("git://", 0) == 0 ||
           s.rfind("ssh://", 0) == 0;
}

} // namespace

int cmd_copy(const std::vector<std::string>& args) {
    if (args.empty() || args.size() > 2) {
        std::cerr << "kit copy: usage: kit copy <source-repo> [dest]\n";
        return 1;
    }

    const std::string& source_arg = args[0];
    if (looks_like_url(source_arg)) {
        std::cerr << "kit copy: cloning from a URL isn't implemented -- kit doesn't speak a git "
                     "network protocol yet\n";
        std::cerr << "pass a local kit repository's path instead\n";
        return 1;
    }

    const fs::path source_root = fs::absolute(source_arg);
    const fs::path source_kit = source_root / ".kit";
    if (!fs::exists(source_kit)) {
        std::cerr << "kit copy: " << source_arg << " is not a kit repository (no .kit there)\n";
        return 1;
    }

    const fs::path dest_root = args.size() == 2 ? fs::path(args[1])
                                                 : fs::current_path() / source_root.filename();
    if (fs::exists(dest_root)) {
        std::cerr << "kit copy: " << dest_root.string() << " already exists\n";
        return 1;
    }

    std::error_code ec;
    fs::create_directories(dest_root, ec);
    if (ec) {
        std::cerr << "kit copy: could not create " << dest_root.string() << ": " << ec.message()
                  << "\n";
        return 1;
    }

    fs::copy(source_kit, dest_root / ".kit", fs::copy_options::recursive, ec);
    if (ec) {
        std::cerr << "kit copy: could not copy repository data: " << ec.message() << "\n";
        return 1;
    }

    const fs::path dest_kit = dest_root / ".kit";

    // Check out HEAD into the new working directory so `kit copy` leaves a
    // ready-to-use checkout, not just a bare object store.
    const auto head = kit::resolve_head(dest_kit);
    std::string tree_hash;
    if (head) {
        kit::ObjectType type;
        std::string payload;
        if (kit::read_object(dest_kit, *head, type, payload) && type == kit::ObjectType::Commit) {
            if (const auto commit = kit::parse_commit(payload)) {
                tree_hash = commit->tree;
            }
        }
    }

    kit::restore_tree(dest_kit, tree_hash, dest_root);

    kit::Index index;
    for (const auto& [path, entry] : kit::flatten_tree(dest_kit, tree_hash)) {
        kit::IndexEntry ie;
        ie.hash = entry.hash;
        ie.size = entry.size;
        ie.path = path;
        index.stage(ie);
    }
    index.save(dest_kit);

    std::cout << "copied " << source_arg << " into " << dest_root.string() << "\n";
    return 0;
}
