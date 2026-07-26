#include "command.h"
#include "commit.h"
#include "dirty_check.h"
#include "index.h"
#include "object_store.h"
#include "park.h"
#include "refs.h"
#include "tree.h"
#include "tree_walk.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace {

std::string head_tree_hash(const fs::path& kit_dir) {
    auto head = kit::resolve_head(kit_dir);
    if (!head) return "";
    kit::ObjectType type;
    std::string payload;
    if (!kit::read_object(kit_dir, *head, type, payload) || type != kit::ObjectType::Commit) return "";
    const auto commit = kit::parse_commit(payload);
    return commit ? commit->tree : "";
}

std::string format_time(std::int64_t epoch) {
    const std::time_t t = static_cast<std::time_t>(epoch);
    std::tm tm{};
    localtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}

int park_list(const fs::path& kit_dir) {
    kit::ParkList parks;
    if (!parks.load(kit_dir)) {
        std::cerr << "kit park: could not read " << (kit_dir / "parks").string() << "\n";
        return 1;
    }

    if (parks.empty()) {
        std::cout << "no parks\n";
        return 0;
    }

    // Newest first, so park@{0} is the most recent -- like git stash.
    const auto& all = parks.all();
    for (std::size_t i = all.size(); i-- > 0;) {
        const kit::Park& p = all[i];
        std::cout << "park@{" << (all.size() - 1 - i) << "}  " << format_time(p.created_at)
                  << "  " << p.message << "  (" << p.entries.size() << " entries)\n";
    }
    return 0;
}

int park_pop(const fs::path& kit_dir, const fs::path& root) {
    kit::ParkList parks;
    if (!parks.load(kit_dir)) {
        std::cerr << "kit park: could not read " << (kit_dir / "parks").string() << "\n";
        return 1;
    }
    if (parks.empty()) {
        std::cerr << "kit park pop: no parks to pop\n";
        return 1;
    }

    // Popping overwrites working-directory files, so refuse if there's
    // unsaved work that could get silently clobbered -- same guard as
    // switch/join/undo.
    if (kit::has_uncommitted_changes(kit_dir, root)) {
        std::cerr << "kit park pop: you have unsaved changes -- 'kit save' them first, they "
                     "could get overwritten\n";
        return 1;
    }

    auto park = parks.pop(); // removes it from the list

    for (const auto& e : park->entries) {
        const fs::path abs = root / e.path;
        std::error_code ec;
        if (e.is_dir) {
            fs::create_directories(abs, ec);
            continue;
        }
        kit::ObjectType type;
        std::string content;
        if (!kit::read_object(kit_dir, e.hash, type, content) || type != kit::ObjectType::Blob) {
            std::cerr << "kit park pop: missing blob for " << e.path << " -- skipped\n";
            continue;
        }
        fs::create_directories(abs.parent_path(), ec);
        std::ofstream out(abs, std::ios::binary | std::ios::trunc);
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
    }

    if (!parks.save(kit_dir)) {
        std::cerr << "kit park: could not write " << (kit_dir / "parks").string() << "\n";
        return 1;
    }

    std::cout << "popped park@{0}: " << park->message << "\n";
    return 0;
}

} // namespace

int cmd_park(const std::vector<std::string>& args) {
    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit park: not a kit repository (no .kit here)\n";
        return 1;
    }

    if (!args.empty() && args[0] == "list") {
        if (args.size() > 1) {
            std::cerr << "kit park list: takes no arguments\n";
            return 1;
        }
        return park_list(kit_dir);
    }

    if (!args.empty() && args[0] == "pop") {
        if (args.size() > 1) {
            std::cerr << "kit park pop: takes no arguments\n";
            return 1;
        }
        return park_pop(kit_dir, root);
    }

    // Everything else is the park message.
    std::string message;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) {
            message += " ";
        }
        message += args[i];
    }
    if (message.empty()) {
        message = "parked work";
    }

    kit::ParkList parks;
    if (!parks.load(kit_dir)) {
        std::cerr << "kit park: could not read " << (kit_dir / "parks").string() << "\n";
        return 1;
    }

    const auto tree = kit::build_tree(root);

    kit::Park park;
    park.created_at = static_cast<std::int64_t>(std::time(nullptr));
    park.message = message;
    park.entries = kit::flatten(kit_dir, root, *tree);

    parks.push(std::move(park));
    if (!parks.save(kit_dir)) {
        std::cerr << "kit park: could not write " << (kit_dir / "parks").string() << "\n";
        return 1;
    }

    // The whole point of parking is to set the working directory aside --
    // everything on disk just got captured above, so it's safe to wipe it
    // and restore exactly what the last save had.
    const std::string head_tree = head_tree_hash(kit_dir);
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(root, ec)) {
        if (entry.path().filename() == ".kit") continue;
        fs::remove_all(entry.path(), ec);
    }
    kit::restore_tree(kit_dir, head_tree, root);

    kit::Index index;
    for (const auto& [path, e] : kit::flatten_tree(kit_dir, head_tree)) {
        kit::IndexEntry ie;
        ie.hash = e.hash;
        ie.size = e.size;
        ie.path = path;
        index.stage(ie);
    }
    index.save(kit_dir);

    std::cout << "parked as park@{0}: " << message << "\n";
    return 0;
}
