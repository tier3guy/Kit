#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace kit {

// The three kinds of object kit stores under .kit/objects. Every object is
// content-addressed: its hash is a function of (type, payload), so writing
// identical content twice always yields the same hash and is a cheap no-op
// the second time.
enum class ObjectType { Blob, Tree, Commit };

std::string object_type_name(ObjectType type);
std::optional<ObjectType> object_type_from_name(const std::string& name);

// Computes the hash an object of `type` and `payload` would have, without
// writing anything to disk. write_object uses this internally; callers that
// only need to *compare* against stored hashes (status, diff) use it
// directly so checking status never pollutes the object store.
std::string hash_for(ObjectType type, const std::string& payload);

// Writes `payload` as an object of `type` and returns its hash. Storage
// format mirrors git: "<type> <size>\0<payload>", hashed as a whole -- the
// header is what makes a tree and a blob with identical bytes hash
// differently.
std::string write_object(const std::filesystem::path& kit_dir, ObjectType type,
                          const std::string& payload);

// Reads an object back by hash. Returns false if it doesn't exist or its
// header doesn't parse.
bool read_object(const std::filesystem::path& kit_dir, const std::string& hash,
                  ObjectType& type_out, std::string& payload_out);

// Path on disk for a given hash: objects/<first 2 chars>/<remaining 38>.
std::filesystem::path object_path(const std::filesystem::path& kit_dir, const std::string& hash);

} // namespace kit
