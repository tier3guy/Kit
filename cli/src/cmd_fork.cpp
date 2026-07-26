#include "command.h"
#include "merge_state.h"
#include "refs.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int cmd_fork(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cerr << "kit fork: usage: kit fork <name>\n";
        return 1;
    }

    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit fork: not a kit repository (no .kit here)\n";
        return 1;
    }

    if (kit::merge_in_progress(kit_dir)) {
        std::cerr << "kit fork: a join is paused with conflicts -- resolve it or 'kit join "
                     "--abort' first\n";
        return 1;
    }

    const std::string& name = args[0];
    if (kit::branch_exists(kit_dir, name)) {
        std::cerr << "kit fork: '" << name << "' already exists\n";
        return 1;
    }

    const auto current = kit::resolve_head(kit_dir);
    if (!current) {
        std::cerr << "kit fork: no saves yet -- nothing to fork from\n";
        return 1;
    }

    kit::write_branch(kit_dir, name, *current);
    kit::attach_head_to_branch(kit_dir, name);

    std::cout << "forked '" << name << "' at " << current->substr(0, 7) << ", now on '" << name
               << "'\n";
    return 0;
}
