#pragma once

#include <string>
#include <vector>

// Every kit subcommand takes the argument list *after* the command name
// and returns the process exit code.
int cmd_new(const std::vector<std::string>& args);
int cmd_park(const std::vector<std::string>& args);
int cmd_tree(const std::vector<std::string>& args);
