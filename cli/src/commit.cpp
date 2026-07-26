#include "commit.h"

#include <sstream>

namespace kit {

std::string serialize_commit(const CommitData& commit) {
    std::ostringstream out;
    out << "tree " << commit.tree << "\n";
    for (const auto& parent : commit.parents) {
        out << "parent " << parent << "\n";
    }
    out << "time " << commit.time << "\n";
    out << "\n"; // blank line separates headers from the free-form message
    out << commit.message;
    return out.str();
}

std::optional<CommitData> parse_commit(const std::string& payload) {
    std::istringstream in(payload);
    CommitData commit;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) {
            break;
        }
        std::istringstream ls(line);
        std::string tag;
        ls >> tag;
        if (tag == "tree") {
            ls >> commit.tree;
        } else if (tag == "parent") {
            std::string parent;
            ls >> parent;
            commit.parents.push_back(parent);
        } else if (tag == "time") {
            ls >> commit.time;
        } else {
            return std::nullopt; // unknown header -- refuse to guess
        }
    }

    if (commit.tree.empty()) {
        return std::nullopt;
    }

    std::ostringstream msg;
    msg << in.rdbuf();
    commit.message = msg.str();
    return commit;
}

} // namespace kit
