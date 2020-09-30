#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include "operations/operations.hpp"
#include<unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMD_SIZE 256

int forking(){
    pid_t parent = getpid();
    pid_t pid = fork();

    if (pid == -1)
    {
        std::cerr << "Failed to fork\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        std::cout << "Parent: " << parent << ", child: " << pid << std::endl;
        int status;
        waitpid(pid, &status, 0);
        std::cout << "child exited with code" << status << std::endl;
    }
    else
    {
        //TODO
    }
    return 0;
}

int cd(const char* path){
    if (chdir(path) != 0)
        std::cerr << "cd: " << path << ": No such file or directory" << std::endl;

    return 0;
}

int main(int argc, char **argv) {

    while ( true ){
        std::cout << getcwd(NULL, 0) << std::endl;
        std::string cmd;
        std::getline(std::cin, cmd);
        const char* cpath = cmd.c_str();
        cd(cpath);
        std::cout << getcwd(NULL, 0) << std::endl;
        return 0;
    }

}