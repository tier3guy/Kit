#include "tree.h"

#include <algorithm>
#include <ostream>

namespace fs = std::filesystem;

namespace kit {

namespace {

void render(const Node& node, std::ostream& out, int depth) {
    for (const auto& child : node.children) {
        out << std::string(static_cast<std::size_t>(depth) * 2, ' ') << child->name;
        if (child->is_tree()) {
            out << "/\n";
            render(*child, out, depth + 1);
        } else {
            out << "  (" << child->size << " bytes)\n";
        }
    }
}

} // namespace

std::unique_ptr<Node> build_tree(const fs::path& root) {
    auto node = std::make_unique<Node>();
    node->name = root.filename().string();
    node->kind = NodeKind::Tree;

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(root, ec)) {
        const std::string name = entry.path().filename().string();
        if (name == ".kit") {
            continue;
        }

        if (entry.is_directory()) {
            node->children.push_back(build_tree(entry.path()));
        } else if (entry.is_regular_file()) {
            auto blob = std::make_unique<Node>();
            blob->name = name;
            blob->kind = NodeKind::Blob;
            blob->size = entry.file_size();
            node->children.push_back(std::move(blob));
        }
    }

    // Deterministic order: directories and files interleaved, sorted by name.
    std::sort(node->children.begin(), node->children.end(),
              [](const auto& a, const auto& b) { return a->name < b->name; });

    return node;
}

void print_tree(const Node& node, std::ostream& out) {
    render(node, out, 0);
}

} // namespace kit
