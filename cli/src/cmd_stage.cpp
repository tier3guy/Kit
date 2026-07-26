#include "command.h"
#include "index.h"
#include "object_store.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace {

std::string read_file(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void stage_file(const fs::path& root, const fs::path& kit_dir, const fs::path& file,
                 kit::Index& index, int& count) {
    const std::string content = read_file(file);
    const std::string hash = kit::write_object(kit_dir, kit::ObjectType::Blob, content);

    kit::IndexEntry entry;
    entry.hash = hash;
    entry.size = content.size();
    entry.path = fs::relative(file, root).generic_string();

    index.stage(std::move(entry));
    ++count;
}

void stage_recursive(const fs::path& root, const fs::path& kit_dir, const fs::path& dir,
                      kit::Index& index, int& count) {
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (entry.path().filename() == ".kit") {
            continue;
        }
        if (entry.is_directory()) {
            stage_recursive(root, kit_dir, entry.path(), index, count);
        } else if (entry.is_regular_file()) {
            stage_file(root, kit_dir, entry.path(), index, count);
        }
    }
}

} // namespace

int cmd_stage(const std::vector<std::string>& args) {
    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit stage: not a kit repository (no .kit here)\n";
        return 1;
    }

    if (args.empty()) {
        std::cerr << "kit stage: nothing to stage (try 'kit stage .')\n";
        return 1;
    }

    kit::Index index;
    if (!index.load(kit_dir)) {
        std::cerr << "kit stage: could not read " << (kit_dir / "index").string() << "\n";
        return 1;
    }

    int count = 0;
    for (const std::string& arg : args) {
        const fs::path target = (arg == ".") ? root : root / arg;

        if (!fs::exists(target)) {
            std::cerr << "kit stage: " << arg << ": no such file or directory\n";
            return 1;
        }

        if (fs::is_directory(target)) {
            stage_recursive(root, kit_dir, target, index, count);
        } else {
            stage_file(root, kit_dir, target, index, count);
        }
    }

    if (!index.save(kit_dir)) {
        std::cerr << "kit stage: could not write " << (kit_dir / "index").string() << "\n";
        return 1;
    }

    std::cout << "staged " << count << " file" << (count == 1 ? "" : "s") << "\n";
    return 0;
}
