#include "refs.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace kit {

namespace {

fs::path head_file(const fs::path& kit_dir) {
    return kit_dir / "HEAD";
}

fs::path branch_file(const fs::path& kit_dir, const std::string& name) {
    return kit_dir / "refs" / "heads" / name;
}

fs::path tag_file(const fs::path& kit_dir, const std::string& name) {
    return kit_dir / "refs" / "tags" / name;
}

std::string read_whole_line(const fs::path& path) {
    std::ifstream in(path);
    std::string line;
    std::getline(in, line);
    return line;
}

void write_whole_line(const fs::path& path, const std::string& line) {
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::trunc);
    out << line << "\n";
}

const std::string kSymbolicPrefix = "ref: refs/heads/";

std::vector<std::string> list_ref_dir(const fs::path& dir) {
    std::vector<std::string> names;
    std::error_code ec;
    if (!fs::exists(dir, ec)) {
        return names;
    }
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (entry.is_regular_file()) {
            names.push_back(entry.path().filename().string());
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

} // namespace

HeadState read_head_state(const fs::path& kit_dir) {
    const std::string line = read_whole_line(head_file(kit_dir));

    if (line.rfind(kSymbolicPrefix, 0) == 0) {
        HeadState state;
        state.detached = false;
        state.branch = line.substr(kSymbolicPrefix.size());
        return state;
    }

    HeadState state;
    state.detached = true;
    state.commit_hash = line;
    return state;
}

std::optional<std::string> resolve_head(const fs::path& kit_dir) {
    const HeadState state = read_head_state(kit_dir);
    if (state.detached) {
        return state.commit_hash.empty() ? std::nullopt : std::make_optional(state.commit_hash);
    }
    return read_branch(kit_dir, state.branch);
}

void attach_head_to_branch(const fs::path& kit_dir, const std::string& branch) {
    write_whole_line(head_file(kit_dir), kSymbolicPrefix + branch);
}

void detach_head_to_commit(const fs::path& kit_dir, const std::string& commit_hash) {
    write_whole_line(head_file(kit_dir), commit_hash);
}

bool branch_exists(const fs::path& kit_dir, const std::string& name) {
    return fs::exists(branch_file(kit_dir, name));
}

std::optional<std::string> read_branch(const fs::path& kit_dir, const std::string& name) {
    const fs::path path = branch_file(kit_dir, name);
    if (!fs::exists(path)) {
        return std::nullopt;
    }
    const std::string hash = read_whole_line(path);
    if (hash.empty()) {
        return std::nullopt;
    }
    return hash;
}

void write_branch(const fs::path& kit_dir, const std::string& name, const std::string& commit_hash) {
    write_whole_line(branch_file(kit_dir, name), commit_hash);
}

std::vector<std::string> list_branches(const fs::path& kit_dir) {
    return list_ref_dir(kit_dir / "refs" / "heads");
}

bool tag_exists(const fs::path& kit_dir, const std::string& name) {
    return fs::exists(tag_file(kit_dir, name));
}

std::optional<std::string> read_tag(const fs::path& kit_dir, const std::string& name) {
    const fs::path path = tag_file(kit_dir, name);
    if (!fs::exists(path)) {
        return std::nullopt;
    }
    const std::string hash = read_whole_line(path);
    if (hash.empty()) {
        return std::nullopt;
    }
    return hash;
}

void write_tag(const fs::path& kit_dir, const std::string& name, const std::string& commit_hash) {
    write_whole_line(tag_file(kit_dir, name), commit_hash);
}

std::vector<std::string> list_tags(const fs::path& kit_dir) {
    return list_ref_dir(kit_dir / "refs" / "tags");
}

std::optional<std::string> resolve_ref(const fs::path& kit_dir, const std::string& ref) {
    if (ref == "HEAD") {
        return resolve_head(kit_dir);
    }
    if (auto branch = read_branch(kit_dir, ref)) {
        return branch;
    }
    if (auto tag = read_tag(kit_dir, ref)) {
        return tag;
    }

    // Otherwise treat it as a commit hash, full or a unique prefix.
    if (ref.size() < 4 || ref.size() > 40) {
        return std::nullopt;
    }
    if (ref.find_first_not_of("0123456789abcdef") != std::string::npos) {
        return std::nullopt;
    }

    const fs::path objects_dir = kit_dir / "objects";
    const std::string dir_part = ref.substr(0, 2);
    const std::string rest_part = ref.substr(2);

    std::vector<std::string> matches;
    std::error_code ec;
    for (const auto& dir_entry : fs::directory_iterator(objects_dir, ec)) {
        if (!dir_entry.is_directory()) continue;
        const std::string dir_name = dir_entry.path().filename().string();
        if (dir_name.size() != 2 || (ref.size() > 2 && dir_name != dir_part)) continue;
        for (const auto& file_entry : fs::directory_iterator(dir_entry.path(), ec)) {
            const std::string file_name = file_entry.path().filename().string();
            const std::string full = dir_name + file_name;
            if (ref.size() <= 2 || file_name.rfind(rest_part, 0) == 0) {
                if (full.rfind(ref, 0) == 0) {
                    matches.push_back(full);
                }
            }
        }
    }

    if (matches.size() == 1) {
        return matches[0];
    }
    return std::nullopt; // no match or ambiguous prefix
}

} // namespace kit
