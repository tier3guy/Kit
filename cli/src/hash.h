#pragma once

#include <string>

namespace kit {

// Returns the 40-character lowercase hex SHA-1 digest of `data`. This is the
// hash kit uses to content-address every object it stores.
std::string sha1_hex(const std::string& data);

} // namespace kit
