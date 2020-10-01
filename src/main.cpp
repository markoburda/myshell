#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include "operations/operations.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <a.out.h>
#include <fstream>
#include "helper_funcs.hpp"
extern char **environ;


int myscript(const std::vector<std::string>& files, bool forking=1){
    for (auto& filename : files) {
        std::ifstream script_file(filename);
        std::string line;
        while (std::getline(script_file, line)) {
            std::vector<std::string> words;
            std::istringstream stream(line);
            std::string word;
            while (stream >> word) {
                if (word[0] == '#') {
                    break;
                } else if (word.find("=") != std::string::npos) {
                    operations::mexport(std::vector<std::string>{word}, 0);
                    continue;
                } else {
                    words.push_back(word);
                }
            }
            if (forking){
                pid_t pid = fork();
                if (pid == -1) {
                    std::cerr << "Failed to fork()" << std::endl;
                    return EXIT_FAILURE;
                } else if (pid > 0) {
                    int status;
                    waitpid(pid, &status, 0);
                } else {
                    std::vector<const char*> arg_for_c;
                    for(const auto& s: words)arg_for_c.push_back(s.c_str());
                    arg_for_c.push_back(nullptr);
                    execvp(words[0].c_str(), const_cast<char* const*>(arg_for_c.data()));
                    std::cerr << "Failed to execve()" << std::endl;
                    return EXIT_FAILURE;
                }
            } else {
                std::vector<const char*> arg_for_c;
                for(const auto& s: words)arg_for_c.push_back(s.c_str());
                arg_for_c.push_back(nullptr);
                execve(words[0].c_str(), const_cast<char* const*>(arg_for_c.data()), environ);
                std::cerr << "Failed to execve()" << std::endl;
                return EXIT_FAILURE;
            }
        }
    }
    return 0;

        
}

int main(int argc, char **argv) {
    if (argc > 1) {
        std::vector<std::string> args;
        for (size_t i = 1; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        myscript(args);
        exit(EXIT_SUCCESS);
    }

    typedef int (*pfunc)(std::vector<std::string>, bool);
    std::map<std::string, pfunc> internal_commands = {
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
        int position = program.find_last_of(".");
        std::string ext = program.substr(position+1);
        if (internal_commands.find(program) != internal_commands.end()) {
            if (program.compare("merrno")==0) {
                if (help){
                    std::cout << "Print error code of last command\nUsage: merrno [-h, --help]\n -h, --help\t Print this help message" << std::endl;        
                } else {
                    std::cout << err << std::endl;
                }
            } else {
            err = (*internal_commands[program])(args, help);
            }
        } else if (program.compare(".")==0) {
            std::cout << program << std::endl;
            myscript(args, 0);
        } else if (ext.find("sh") != std::string::npos) {
            myscript(args);
        } else {
            pid_t pid = fork();
            if (pid == -1) {
                std::cerr << "Failed to fork()" << std::endl;
                return EXIT_FAILURE;
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
            } else {
                std::vector<const char*> arg_for_c;
                arg_for_c.push_back(program.c_str());
                for(const auto& s: args)arg_for_c.push_back(s.c_str());
                arg_for_c.push_back(nullptr);
                execve(program.c_str(), const_cast<char* const*>(arg_for_c.data()), environ);
                std::cerr << "Failed to execve()" << std::endl;
                return EXIT_FAILURE;
            }        
            
        }  
    }
    return 0;
}