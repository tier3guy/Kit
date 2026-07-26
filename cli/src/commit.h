#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace kit {

// The payload stored inside a Commit object. Zero parents means the first
// commit in the repository; one parent is the common case; two or more
// parents means this is a merge commit (kit join).
struct CommitData {
    std::string tree; // root tree hash
    std::vector<std::string> parents;
    std::int64_t time = 0; // unix epoch seconds
    std::string message;

    bool is_merge() const { return parents.size() > 1; }
};

std::string serialize_commit(const CommitData& commit);
std::optional<CommitData> parse_commit(const std::string& payload);

} // namespace kit
