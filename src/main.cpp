#include <iostream>
#include <string>
#include "operations/operations.hpp"
#include <unistd.h>
#include "helper_funcs.hpp"
extern char **environ;





int main(int argc, char **argv) {
    if (argc > 1) {
        std::vector<std::string> args;
    for (size_t i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    //TODO
    exit(EXIT_SUCCESS);
    }

    typedef int (*pfunc)(std::vector<std::string>, bool);
    std::map<std::string, pfunc> commands = {
        {"merrno", operations::merrno},
        {"mecho", operations::mecho},
        {"mexport", operations::mexport},
        {"mexit", operations::mexit},
        {"mpwd", operations::mpwd},
        {"mcd", operations::mcd}
    };
    int err = 0;
    std::string input;
    while (true) {
        std::cout << get_current_dir() << "$ ";
        getline(std::cin, input);
        std::string program;
        std::vector<std::string> args;
        bool help;
        parse_args(input, program, args, help);
        if (commands.find(program) != commands.end()) {
            if (program.compare("merrno")==0) {
                std::cout << err << std::endl;
            } else {
            err = (*commands[program])(args, help);
            }
        } else {
            std::cout << "Unknown command\n";
            continue;
        }   
        
    }
}



