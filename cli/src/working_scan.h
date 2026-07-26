#pragma once

#include "tree_walk.h"

#include <filesystem>

namespace kit {

// Walks the working directory (skipping .kit) and computes what each
// file's blob hash *would* be, without writing anything to the object
// store. This lets status/diff compare working-directory content against
// staged or committed hashes cheaply and without side effects.
FlatTree scan_working_tree(const std::filesystem::path& root);

} // namespace kit
