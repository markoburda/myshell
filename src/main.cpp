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

std::vector<char *> str_to_argv(std::string str){
    std::vector<char *> args;
    std::istringstream iss(str);
    std::string token;

    while(iss >> token) {
        char *arg = new char[token.size() + 1];
        copy(token.begin(), token.end(), arg);
        arg[token.size()] = '\0';
        args.push_back(arg);
    }
    args.push_back(nullptr);

//    for(auto &kv: args){
//        std::cout << "Arg: " << kv << std::endl;
//    }

//    for(size_t i = 0; i < args.size(); i++)
//        delete[] args[i];
//    execv("myshell", &args[0]);
    return args;
}

int myscript(const std::string& filename){
    std::ifstream input(filename);
    for( std::string line; getline( input, line ); )
    {
        if(line[0] != '#'){
            printf("Line: %s\n", line.c_str());
            pid_t parent = getpid();
            pid_t pid = fork();
            if (pid == -1)
                {
                    std::cerr << "Failed to fork\n" << std::endl;
                    return -1;
                }
            printf("Child PID: %d", pid);
            execv("myshell", &str_to_argv(line)[0]);
            int status;
//            waitpid(pid, &status, 0);
        }
    }
    return 0;
}

int mcd(const char* path){
    if (chdir(path) != 0)
        std::cerr << "cd: " << path << ": No such file or directory" << std::endl;
    return 0;
}

int mpwd(){
    std::cout << getcwd(NULL, 0) << "$ ";
    return 0;
}

//One version of loop
//    while(true) {
////        mpwd();
//        printf("main PID: %d\n", getpid());
//        for(int i = 0; i < argc; i++){
//            printf("arg: %s\n", argv[i]);
//        }
//        if (strcmp(argv[0], "echo") == 0) {
//            printf("echo detected, exiting\n");
//            return EXIT_SUCCESS;
//        }
//        else{
//            myscript("test.msh");
//        }
//        printf("Back to main process");
//        return 0;
//    }

int main(int argc, char **argv) {
    if (argc > 1) {
        std::vector<std::string> args;
    for (size_t i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
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
        getline(std::cin,    input);
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