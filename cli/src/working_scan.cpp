#include "working_scan.h"

#include "object_store.h"

#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace kit {

namespace {

std::string read_file(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void scan_into(const fs::path& root, const fs::path& dir, const std::string& prefix,
               FlatTree& out) {
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (entry.path().filename() == ".kit") {
            continue;
        }
        const std::string path = prefix.empty() ? entry.path().filename().string()
                                                  : prefix + "/" + entry.path().filename().string();
        if (entry.is_directory()) {
            scan_into(root, entry.path(), path, out);
        } else if (entry.is_regular_file()) {
            const std::string content = read_file(entry.path());
            out[path] = FlatEntry{hash_for(ObjectType::Blob, content), content.size()};
        }
    }
}

} // namespace

FlatTree scan_working_tree(const fs::path& root) {
    FlatTree out;
    scan_into(root, root, "", out);
    return out;
}

} // namespace kit
