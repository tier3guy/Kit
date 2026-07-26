#include "diff.h"

#include <algorithm>
#include <vector>

namespace kit {

std::vector<std::string> split_lines(const std::string& text) {
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t nl = text.find('\n', start);
        if (nl == std::string::npos) {
            if (start < text.size()) {
                lines.push_back(text.substr(start));
            }
            break;
        }
        lines.push_back(text.substr(start, nl - start));
        start = nl + 1;
    }
    return lines;
}

std::vector<DiffLine> diff_lines(const std::vector<std::string>& a,
                                  const std::vector<std::string>& b) {
    const std::size_t n = a.size();
    const std::size_t m = b.size();

    // Standard LCS length table, dp[i][j] = LCS length of a[i..) and b[j..).
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));
    for (std::size_t i = n; i-- > 0;) {
        for (std::size_t j = m; j-- > 0;) {
            if (a[i] == b[j]) {
                dp[i][j] = dp[i + 1][j + 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i + 1][j], dp[i][j + 1]);
            }
        }
    }

    std::vector<DiffLine> out;
    std::size_t i = 0, j = 0;
    while (i < n && j < m) {
        if (a[i] == b[j]) {
            out.push_back({DiffOp::Equal, a[i]});
            ++i;
            ++j;
        } else if (dp[i + 1][j] >= dp[i][j + 1]) {
            out.push_back({DiffOp::Remove, a[i]});
            ++i;
        } else {
            out.push_back({DiffOp::Add, b[j]});
            ++j;
        }
    }
    while (i < n) {
        out.push_back({DiffOp::Remove, a[i]});
        ++i;
    }
    while (j < m) {
        out.push_back({DiffOp::Add, b[j]});
        ++j;
    }

    return out;
}

} // namespace kit
