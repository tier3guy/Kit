#include "ancestry.h"

#include "commit.h"
#include "object_store.h"

#include <deque>
#include <map>

namespace fs = std::filesystem;

namespace kit {

namespace {

// hash -> BFS distance from `start` (0 == start itself).
std::map<std::string, int> ancestors_with_distance(const fs::path& kit_dir,
                                                     const std::string& start) {
    std::map<std::string, int> distance;
    std::deque<std::string> queue{start};
    distance[start] = 0;

    while (!queue.empty()) {
        const std::string current = queue.front();
        queue.pop_front();

        ObjectType type;
        std::string payload;
        if (!read_object(kit_dir, current, type, payload) || type != ObjectType::Commit) {
            continue;
        }
        const auto commit = parse_commit(payload);
        if (!commit) {
            continue;
        }

        for (const std::string& parent : commit->parents) {
            if (distance.find(parent) == distance.end()) {
                distance[parent] = distance[current] + 1;
                queue.push_back(parent);
            }
        }
    }

    return distance;
}

} // namespace

bool is_ancestor(const fs::path& kit_dir, const std::string& ancestor, const std::string& commit) {
    if (ancestor == commit) {
        return true;
    }
    const auto distances = ancestors_with_distance(kit_dir, commit);
    return distances.find(ancestor) != distances.end();
}

std::optional<std::string> merge_base(const fs::path& kit_dir, const std::string& a,
                                       const std::string& b) {
    const auto dist_a = ancestors_with_distance(kit_dir, a);
    const auto dist_b = ancestors_with_distance(kit_dir, b);

    std::optional<std::string> best;
    int best_total = -1;

    for (const auto& [hash, da] : dist_a) {
        const auto it = dist_b.find(hash);
        if (it == dist_b.end()) {
            continue;
        }
        const int total = da + it->second;
        if (best_total < 0 || total < best_total) {
            best_total = total;
            best = hash;
        }
    }

    return best;
}

} // namespace kit
