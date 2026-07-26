#pragma once

#include <string>
#include <vector>

namespace kit {

enum class DiffOp { Equal, Add, Remove };

struct DiffLine {
    DiffOp op;
    std::string text;
};

// Splits text on '\n' into lines (no trailing empty line for a final \n).
std::vector<std::string> split_lines(const std::string& text);

// Line-based diff via longest common subsequence -- the same idea behind
// `diff`/`git diff`, just without the surrounding hunk-header bookkeeping.
std::vector<DiffLine> diff_lines(const std::vector<std::string>& a, const std::vector<std::string>& b);

} // namespace kit
