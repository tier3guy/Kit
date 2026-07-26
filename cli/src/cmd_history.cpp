#include "command.h"
#include "commit.h"
#include "object_store.h"
#include "refs.h"

#include <ctime>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace {

std::string format_time(std::int64_t epoch) {
    const std::time_t t = static_cast<std::time_t>(epoch);
    std::tm tm{};
    localtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}

} // namespace

int cmd_history(const std::vector<std::string>& args) {
    if (!args.empty()) {
        std::cerr << "kit history: takes no arguments\n";
        return 1;
    }

    const fs::path root = fs::current_path();
    const fs::path kit_dir = root / ".kit";
    if (!fs::exists(kit_dir)) {
        std::cerr << "kit history: not a kit repository (no .kit here)\n";
        return 1;
    }

    auto current = kit::resolve_head(kit_dir);
    if (!current) {
        std::cout << "no history yet -- run 'kit save' to make your first one\n";
        return 0;
    }

    bool first = true;
    while (current) {
        kit::ObjectType type;
        std::string payload;
        if (!kit::read_object(kit_dir, *current, type, payload) || type != kit::ObjectType::Commit) {
            std::cerr << "kit history: corrupt commit " << *current << "\n";
            return 1;
        }

        const auto commit = kit::parse_commit(payload);
        if (!commit) {
            std::cerr << "kit history: could not parse commit " << *current << "\n";
            return 1;
        }

        if (!first) {
            std::cout << "\n";
        }
        first = false;

        std::cout << "save " << current->substr(0, 7);
        if (commit->is_merge()) {
            std::cout << "  (merge)";
        }
        std::cout << "\n";
        std::cout << "date  " << format_time(commit->time) << "\n\n";
        std::cout << "    " << commit->message << "\n";

        // Follow first-parent only, so history stays a single readable
        // line even across merges (same convention git log uses by default).
        current = commit->parents.empty() ? std::nullopt
                                           : std::make_optional(commit->parents.front());
    }

    return 0;
}
