#include "command.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

int cmd_link(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cerr << "kit link: usage: kit link <url>\n";
        return 1;
    }

    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit link: not a kit repository (no .kit here)\n";
        return 1;
    }

    std::ofstream out(kit_dir / "remote", std::ios::trunc);
    out << args[0] << "\n";

    // kit doesn't speak a network transport -- this is bookkeeping only, so
    // callers of `kit copy` and any future push/pull know where "upstream"
    // is meant to be. Being upfront about that here beats a command that
    // looks like it did more than it did.
    std::cout << "linked remote: " << args[0] << "\n";
    std::cout << "(recorded only -- kit doesn't push or pull over the network yet)\n";
    return 0;
}
