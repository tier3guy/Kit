#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace kit {

enum class NodeKind {
    Blob, // a regular file
    Tree, // a directory
};

// One node in a snapshot of the working directory. A Blob node is a leaf and
// carries the file's size; a Tree node carries its children, sorted by name so
// that the same directory always produces the same tree.
struct Node {
    std::string name;
    NodeKind kind = NodeKind::Blob;

    std::uintmax_t size = 0;                     // Blob only
    std::vector<std::unique_ptr<Node>> children; // Tree only

    bool is_tree() const { return kind == NodeKind::Tree; }
};

// Walks `root` and builds the snapshot tree. The returned node is the root
// directory itself. Skips ".kit" and anything that is neither a regular file
// nor a directory (symlinks, sockets, devices).
std::unique_ptr<Node> build_tree(const std::filesystem::path& root);

// Renders the tree as indented text, one entry per line.
void print_tree(const Node& node, std::ostream& out);

} // namespace kit
