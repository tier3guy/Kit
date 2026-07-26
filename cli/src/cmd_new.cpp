#include "command.h"
#include "refs.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int cmd_new(const std::vector<std::string>& args) {
    if (!args.empty()) {
        std::cerr << "kit new: takes no arguments\n";
        return 1;
    }

    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";

    if (fs::exists(kit_dir)) {
        std::cerr << "kit new: " << kit_dir.string() << " already exists\n";
        return 1;
    }

    std::error_code ec;

    // .kit/objects holds every blob, tree, and commit kit ever records.
    fs::create_directories(kit_dir / "objects", ec);
    if (ec) {
        std::cerr << "kit new: could not create " << kit_dir.string() << ": "
                  << ec.message() << "\n";
        return 1;
    }
    fs::create_directories(kit_dir / "refs" / "heads", ec);
    fs::create_directories(kit_dir / "refs" / "tags", ec);

    // HEAD starts attached to "main". main itself has no ref file yet --
    // it comes into existence on the first save, same as a fresh git repo.
    kit::attach_head_to_branch(kit_dir, "main");

    std::cout << "initialized empty kit repository in " << kit_dir.string() << "\n";
    return 0;
}
