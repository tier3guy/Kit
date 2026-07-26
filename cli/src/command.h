#pragma once

#include <string>
#include <vector>

// Every kit subcommand takes the argument list *after* the command name
// and returns the process exit code.
int cmd_new(const std::vector<std::string>& args);
int cmd_park(const std::vector<std::string>& args);
int cmd_tree(const std::vector<std::string>& args);
int cmd_stage(const std::vector<std::string>& args);
int cmd_save(const std::vector<std::string>& args);
int cmd_history(const std::vector<std::string>& args);
int cmd_check(const std::vector<std::string>& args);
int cmd_compare(const std::vector<std::string>& args);
int cmd_fork(const std::vector<std::string>& args);
int cmd_switch(const std::vector<std::string>& args);
int cmd_join(const std::vector<std::string>& args);
int cmd_undo(const std::vector<std::string>& args);
int cmd_link(const std::vector<std::string>& args);
int cmd_copy(const std::vector<std::string>& args);
int cmd_mark(const std::vector<std::string>& args);
