#include <iostream>
#include <string>
#include <algorithm>
#include <boost/program_options.hpp>
#include "operations/operations.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <a.out.h>
#include <fstream>
#include "helper_funcs.hpp"
extern char **environ;

int fexec(const std::string& program, const std::vector<std::string>& args, int from = -1, int to = -1, int from_d = -1, int to_d = -1, bool background = false){
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Failed to fork()" << std::endl;
        return EXIT_FAILURE;
        
    } else if (!background && pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else if (background && pid > 0){
        signal(SIGCHLD,SIG_IGN);   
    } else {
        if (background) {
            close(0);
            close(1);        
            close(2);
        }    
        if (from != -1) {
            dup2(from, from_d);
        }
            
        if (to != -1) {
            if (to_d == 3){
                dup2(to, 1);
                close(to);
                dup2(1, 2);
            } else {
                dup2(to, to_d);
            }
        }
        std::vector<const char*> arg_for_c;
        for(const auto& s: args)arg_for_c.push_back(s.c_str());
        arg_for_c.push_back(nullptr);
        execvp(program.c_str(), const_cast<char* const*>(arg_for_c.data()));
        std::cerr << "Failed to exxec()" << std::endl;
        exit(1);
        
    } 
    return EXIT_SUCCESS; 
}


int conveyor(std::vector<std::string> args, bool back){
    std::vector<std::vector<std::string>> commands;
    std::vector<std::string> com_args;
    args.push_back("|");
    for (size_t i = 0; i < args.size(); i++) {

        if (args[i] == "|") {
            commands.push_back(com_args);
            com_args.clear();
        } else {
            com_args.push_back(args[i]);
        }
    }
    std::vector<std::vector<int> > pipefd(commands.size()-1 , std::vector<int> (2, 0));
    for (size_t i = 0; i < commands.size() - 1; i++) {
        if (pipe(&pipefd[i][0]) < 0) {
            std::cerr << "Could not init pipe" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    for (size_t i = 0; i < commands.size(); i++) {
        if (i == 0) {
            fexec(commands[i][0], commands[i], -1, pipefd[i][1], -1, 1, back);
            close(pipefd[i][1]);
        } else if (i == commands.size() - 1) {
            fexec(commands[i][0], commands[i], pipefd[i-1][0], -1, 0, -1, back);
            close(pipefd[i-1][0]);
        } else {
            fexec(commands[i][0], commands[i], pipefd[i-1][0], pipefd[i][1], 0, 1, back);
            close(pipefd[i-1][0]);
            close(pipefd[i][1]);
        }
    }
}

int redirect(const std::string& from, const std::string& to, const std::string& op, const std::vector<std::string>& args){
    int fd;
    if (op == "<") {
        fd = open(to.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    } else {
        fd = open(to.c_str(), O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    }
    if (fd < 0) {
        std::cerr << "Can not open the file" << std::endl;
        return EXIT_FAILURE;
    }
    if (op == ">")
        fexec(from, args, -1, fd, -1, 1);
    else if (op == "2>")
        fexec(from, args, -1, fd, -1, 2);
    else if (op == "2>&1"){
        fexec(from, args, -1, fd, -1, 3);
    } else if (op == "&>") {
        fexec(from, args, -1, fd, -1, 3);
    } else if (op == "<"){
        fexec(from, args, fd, -1, 0, -1);    
    }
    close(fd);
    return EXIT_SUCCESS;
}

int process_conv(std::string& program, std::vector<std::string>& args, bool back){
    std::vector<std::string> new_args = std::vector<std::string>{program};
    args.insert(args.begin(), new_args.begin(), new_args.end());
    for (size_t i = 0; i < args.size() - 1; i++) {
        if (args[i] == "|"){
            conveyor(args, back);
        } else if (args[i] == ">") {
            if (args[args.size()-1] == "2>&1") {
                redirect(args[0], args[2], "2>&1", std::vector<std::string> (args.begin(), args.begin()+1));
            } else
                redirect(args[0], args[i+1], args[i], std::vector<std::string> (args.begin(), args.begin()+i));
        } else if (args[i] == "2>") {
            redirect(args[0], args[i+1], args[i], std::vector<std::string> (args.begin(), args.begin()+i));
            break;
        } else if (args[i] == "&>"){
            redirect(args[0], args[i+1], args[i], std::vector<std::string> (args.begin(), args.begin()+1));
            break;
        } else if (args[i] == "<") {
            redirect(args[0], args[i+1], args[i], std::vector<std::string> (args.begin(), args.begin()+1));
            break;
        } 
     }
 }

 



int main(int argc, char **argv) {
    if (argc > 1) {
        std::vector<std::string> args;
        for (size_t i = 1; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        operations::myscript(args);
        exit(EXIT_SUCCESS);
    }
    typedef int (*pfunc)(std::vector<std::string>, bool);
    std::map<std::string, pfunc> internal_commands = {
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
        if (program.compare("merrno")==0) {
            if (help){
                std::cout << "Print error code of last command\nUsage: merrno [-h, --help]\n -h, --help\t Print this help message" << std::endl;        
            } else {
                std::cout << err << std::endl;
            }
            continue;
        }
        if (program.compare("mycat")==0)
            program = "./" + program;
        args = process(args, err);
        if (err){
            continue;
        }
        int position = program.find_last_of(".");
        std::string ext = program.substr(position+1);
        if (internal_commands.find(program) != internal_commands.end()) {
            err = (*internal_commands[program])(args, help);
        } else if (program.compare(".")==0) {
            err = operations::myscript(args, 0);
        } else if (program.find("=") != std::string::npos) {
            err = operations::mexport(std::vector<std::string>{program}, false);
        } else if (ext.find("sh") != std::string::npos) {
            err = operations::myscript(std::vector<std::string>{program});              
        } else {
            bool back = false;
            if ((args.size() > 0) && (args[args.size()-1] == "&")) {
                args.pop_back();
                back = true;
            }
            if ((args.size() > 0) &&((std::find(args.begin(), args.end(), ">") != args.end()) || (std::find(args.begin(), args.end(), "<") != args.end()) || 
               (std::find(args.begin(), args.end(), "|") != args.end()) || (std::find(args.begin(), args.end(), "2>") != args.end()) ||
               (std::find(args.begin(), args.end(), "2>&1") != args.end()) || (std::find(args.begin(), args.end(), "&>") != args.end()))) {
                process_conv(program, args, back);
            } else {
                std::vector<std::string> new_args = std::vector<std::string>{program};
                args.insert(args.begin(), new_args.begin(), new_args.end());
                fexec(program, args, -1, -1, -1, -1, back);
            }
        
        } 
    }
    return 0; 
}
    

