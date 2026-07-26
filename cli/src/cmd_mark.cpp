#include "command.h"
#include "refs.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int cmd_mark(const std::vector<std::string>& args) {
    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit mark: not a kit repository (no .kit here)\n";
        return 1;
    }

    if (args.empty()) {
        const auto marks = kit::list_tags(kit_dir);
        if (marks.empty()) {
            std::cout << "no marks yet\n";
            return 0;
        }
        for (const auto& name : marks) {
            const auto hash = kit::read_tag(kit_dir, name);
            std::cout << name << "  " << (hash ? hash->substr(0, 7) : "?") << "\n";
        }
        return 0;
    }

    if (args.size() != 1) {
        std::cerr << "kit mark: usage: kit mark [<name>]\n";
        return 1;
    }

    const std::string& name = args[0];
    if (kit::tag_exists(kit_dir, name)) {
        std::cerr << "kit mark: '" << name << "' already exists\n";
        return 1;
    }

    const auto current = kit::resolve_head(kit_dir);
    if (!current) {
        std::cerr << "kit mark: no saves yet -- nothing to mark\n";
        return 1;
    }

    kit::write_tag(kit_dir, name, *current);
    std::cout << "marked '" << name << "' at " << current->substr(0, 7) << "\n";
    return 0;
}
