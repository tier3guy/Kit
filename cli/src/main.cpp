#include "command.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

const char* kVersion = "0.2.0";

void print_usage() {
    std::cout << "kit - a version control system\n"
              << "\n"
              << "usage: kit <command> [<args>]\n"
              << "\n"
              << "commands:\n"
              << "  new             start a repo here\n"
              << "  stage <path>    queue changes ('.' for everything)\n"
              << "  save -m <msg>   record a snapshot\n"
              << "  check           what's changed\n"
              << "  history         walk past saves\n"
              << "  compare [a] [b] line-by-line changes\n"
              << "  fork <name>     fork a line of work\n"
              << "  switch <name>   move between branches\n"
              << "  join <branch>   bring branches together\n"
              << "  undo            rewind the pointer\n"
              << "  park            set changes aside\n"
              << "  park list       list the parks, newest first\n"
              << "  park pop        restore the most recent park\n"
              << "  link <url>      connect to a remote\n"
              << "  copy <path>     duplicate a repo\n"
              << "  mark <name>     flag a point in history\n"
              << "  tree            print the working directory as a snapshot tree\n"
              << "  help            show this message\n"
              << "  version         show the kit version\n";
}

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty()) {
        print_usage();
        return 1;
    }

    const std::string& command = args[0];
    const std::vector<std::string> rest(args.begin() + 1, args.end());

    if (command == "new") return cmd_new(rest);
    if (command == "park") return cmd_park(rest);
    if (command == "tree") return cmd_tree(rest);
    if (command == "stage") return cmd_stage(rest);
    if (command == "save") return cmd_save(rest);
    if (command == "history") return cmd_history(rest);
    if (command == "check") return cmd_check(rest);
    if (command == "compare") return cmd_compare(rest);
    if (command == "fork") return cmd_fork(rest);
    if (command == "switch") return cmd_switch(rest);
    if (command == "join") return cmd_join(rest);
    if (command == "undo") return cmd_undo(rest);
    if (command == "link") return cmd_link(rest);
    if (command == "copy") return cmd_copy(rest);
    if (command == "mark") return cmd_mark(rest);

    if (command == "help" || command == "-h" || command == "--help") {
        print_usage();
        return 0;
    }

    if (command == "version" || command == "-v" || command == "--version") {
        std::cout << "kit version " << kVersion << "\n";
        return 0;
    }

    std::cerr << "kit: '" << command << "' is not a kit command. See 'kit help'.\n";
    return 1;
}
