#include "command.h"
#include "tree.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int cmd_tree(const std::vector<std::string>& args) {
    if (!args.empty()) {
        std::cerr << "kit tree: takes no arguments\n";
        return 1;
    }

    const fs::path root = fs::current_path();
    if (!fs::exists(root / ".kit")) {
        std::cerr << "kit tree: not a kit repository (no .kit here)\n";
        return 1;
    }

    const auto tree = kit::build_tree(root);
    kit::print_tree(*tree, std::cout);
    return 0;
}
