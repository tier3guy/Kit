#include "merge_state.h"

#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace kit {

namespace {

fs::path head_path(const fs::path& kit_dir) { return kit_dir / "MERGE_HEAD"; }
fs::path msg_path(const fs::path& kit_dir) { return kit_dir / "MERGE_MSG"; }

std::string read_whole_file(const fs::path& path) {
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

} // namespace

bool merge_in_progress(const fs::path& kit_dir) {
    return fs::exists(head_path(kit_dir));
}

std::optional<std::string> read_merge_head(const fs::path& kit_dir) {
    if (!fs::exists(head_path(kit_dir))) return std::nullopt;
    std::string hash = read_whole_file(head_path(kit_dir));
    while (!hash.empty() && (hash.back() == '\n' || hash.back() == '\r')) hash.pop_back();
    if (hash.empty()) return std::nullopt;
    return hash;
}

std::optional<std::string> read_merge_msg(const fs::path& kit_dir) {
    if (!fs::exists(msg_path(kit_dir))) return std::nullopt;
    std::string msg = read_whole_file(msg_path(kit_dir));
    while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) msg.pop_back();
    if (msg.empty()) return std::nullopt;
    return msg;
}

void write_merge_state(const fs::path& kit_dir, const std::string& other_commit,
                        const std::string& message) {
    std::ofstream head(head_path(kit_dir), std::ios::trunc);
    head << other_commit << "\n";
    std::ofstream msg(msg_path(kit_dir), std::ios::trunc);
    msg << message << "\n";
}

void clear_merge_state(const fs::path& kit_dir) {
    std::error_code ec;
    fs::remove(head_path(kit_dir), ec);
    fs::remove(msg_path(kit_dir), ec);
}

} // namespace kit
