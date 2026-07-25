#include "command.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

const char* kVersion = "0.1.0";

void print_usage() {
    std::cout << "kit - a version control system\n"
              << "\n"
              << "usage: kit <command> [<args>]\n"
              << "\n"
              << "commands:\n"
              << "  new        create a new kit repository\n"
              << "  park       shelve the current working directory\n"
              << "  park list  list the parks, newest first\n"
              << "  tree       print the working directory as a snapshot tree\n"
              << "  help       show this message\n"
              << "  version    show the kit version\n";
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

    if (command == "new") {
        return cmd_new(rest);
    }

    if (command == "park") {
        return cmd_park(rest);
    }

    if (command == "tree") {
        return cmd_tree(rest);
    }

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
