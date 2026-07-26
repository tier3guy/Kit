#include "park.h"

#include "object_store.h"

#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace kit {

namespace {

const char* kHeader = "kit-parks 2";

fs::path parks_file(const fs::path& kit_dir) {
    return kit_dir / "parks";
}

// Messages are stored one per line, so a newline would corrupt the file.
std::string one_line(std::string text) {
    for (char& c : text) {
        if (c == '\n' || c == '\r') {
            c = ' ';
        }
    }
    return text;
}

std::string read_file(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void flatten_into(const fs::path& kit_dir, const fs::path& abs_prefix, const Node& node,
                   const std::string& rel_prefix, std::vector<ParkEntry>& out) {
    for (const auto& child : node.children) {
        const std::string rel_path =
            rel_prefix.empty() ? child->name : rel_prefix + "/" + child->name;
        const fs::path abs_path = abs_prefix / child->name;

        if (child->is_tree()) {
            out.push_back(ParkEntry{true, 0, "", rel_path});
            flatten_into(kit_dir, abs_path, *child, rel_path, out);
        } else {
            const std::string content = read_file(abs_path);
            const std::string hash = write_object(kit_dir, ObjectType::Blob, content);
            out.push_back(ParkEntry{false, child->size, hash, rel_path});
        }
    }
}

} // namespace

std::vector<ParkEntry> flatten(const fs::path& kit_dir, const fs::path& root,
                                const Node& root_node) {
    std::vector<ParkEntry> entries;
    flatten_into(kit_dir, root, root_node, "", entries);
    return entries;
}

void ParkList::push(Park park) {
    parks_.push_back(std::move(park));
}

std::optional<Park> ParkList::pop() {
    if (parks_.empty()) {
        return std::nullopt;
    }
    Park park = std::move(parks_.back());
    parks_.pop_back();
    return park;
}

bool ParkList::load(const fs::path& kit_dir) {
    parks_.clear();

    const fs::path path = parks_file(kit_dir);
    if (!fs::exists(path)) {
        return true; // no parks yet
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
        std::istringstream ls(line);
        std::string tag;
        ls >> tag;

        if (tag == "park") {
            Park park;
            ls >> park.created_at;
            parks_.push_back(std::move(park));
        } else if (parks_.empty()) {
            return false; // entry before any park header
        } else if (tag == "msg") {
            ls >> std::ws;
            std::string message;
            std::getline(ls, message);
            parks_.back().message = message;
        } else if (tag == "d") {
            ls >> std::ws;
            std::string p;
            std::getline(ls, p);
            parks_.back().entries.push_back(ParkEntry{true, 0, "", p});
        } else if (tag == "f") {
            std::string hash;
            std::uintmax_t size = 0;
            ls >> hash >> size >> std::ws;
            std::string p;
            std::getline(ls, p);
            parks_.back().entries.push_back(ParkEntry{false, size, hash, p});
        } else if (!tag.empty()) {
            return false; // unknown tag
        }
    }

    return true;
}

bool ParkList::save(const fs::path& kit_dir) const {
    const fs::path path = parks_file(kit_dir);
    const fs::path tmp = path.string() + ".tmp";

    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) {
            return false;
        }

        out << kHeader << "\n";
        for (const Park& park : parks_) {
            out << "park " << park.created_at << "\n";
            out << "msg " << one_line(park.message) << "\n";
            for (const ParkEntry& e : park.entries) {
                if (e.is_dir) {
                    out << "d " << e.path << "\n";
                } else {
                    out << "f " << e.hash << " " << e.size << " " << e.path << "\n";
                }
            }
        }

        out.flush();
        if (!out) {
            return false;
        }
    }

    // Rename over the real file so a crash mid-write cannot leave the park
    // list truncated.
    std::error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) {
        fs::remove(tmp, ec);
        return false;
    }
    return true;
}

} // namespace kit

