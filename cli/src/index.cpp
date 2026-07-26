#include "index.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace kit {

namespace {

const char* kHeader = "kit-index 1";

fs::path index_file(const fs::path& kit_dir) {
    return kit_dir / "index";
}

} // namespace

void Index::stage(IndexEntry entry) {
    for (auto& existing : entries_) {
        if (existing.path == entry.path) {
            existing = std::move(entry);
            return;
        }
    }
    entries_.push_back(std::move(entry));
    std::sort(entries_.begin(), entries_.end(),
              [](const IndexEntry& a, const IndexEntry& b) { return a.path < b.path; });
}

bool Index::load(const fs::path& kit_dir) {
    entries_.clear();

    const fs::path path = index_file(kit_dir);
    if (!fs::exists(path)) {
        return true; // nothing staged yet
    }

    std::ifstream in(path);
    if (!in) {
        return false;
    }

    std::string line;
    if (!std::getline(in, line) || line != kHeader) {
        return false;
    }

    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream ls(line);
        IndexEntry e;
        ls >> e.mode >> e.hash >> e.size >> std::ws;
        std::getline(ls, e.path);
        if (e.path.empty()) {
            return false;
        }
        entries_.push_back(std::move(e));
    }

    return true;
}

bool Index::save(const fs::path& kit_dir) const {
    const fs::path path = index_file(kit_dir);
    const fs::path tmp = path.string() + ".tmp";

    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) {
            return false;
        }
        out << kHeader << "\n";
        for (const auto& e : entries_) {
            out << e.mode << " " << e.hash << " " << e.size << " " << e.path << "\n";
        }
        out.flush();
        if (!out) {
            return false;
        }
    }

    std::error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) {
        fs::remove(tmp, ec);
        return false;
    }
    return true;
}

} // namespace kit
