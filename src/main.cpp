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
#include <sys/socket.h>
#include <arpa/inet.h>
extern char **environ;

namespace po = boost::program_options;

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
        std::cerr << "Failed to exec()" << std::endl;
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

void process_conv(std::string& program, std::vector<std::string>& args, bool back){
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

 int assign_cmd(std::string& program, std::vector<std::string>& args) {
    std::string str_arg= "";
    str_arg += program;
    for (size_t i = 0; i < args.size(); i++){
        str_arg += " ";
        str_arg = str_arg + args[i];

    }
    for (size_t i = 0; i < str_arg.size(); i++) {
        if (str_arg[i] == '$'){
            str_arg.erase(str_arg.begin(), str_arg.begin()+i+2);
            
            str_arg.erase(std::prev(str_arg.end())); 
        }
        
    }
    std::vector<std::string> args_as = tokenize(str_arg);

    args_as.push_back(">");
    args_as.push_back("./file.txt");
    process_conv(args_as[0], args_as, false);
    std::ifstream ifs("./file.txt");
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
               (std::istreambuf_iterator<char>()));
    std::remove("./file.txt");
    std::string var_name = tokenize(program, "=")[0];
    var_name += "=" + content;
    int err = operations::mexport(std::vector<std::string>{var_name}, false);
    return err;
 }

int process_execute(int argc, char **argv, int sock = 0) {
    if ((sock == 0) && (argc > 1)) {
        std::vector<std::string> args;
        for (size_t i = 1; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        operations::myscript(args);
        exit(EXIT_SUCCESS);
    }
    auto path_ptr = getenv("PATH");
    std::string path;
    if (path_ptr != nullptr)
        path = path_ptr;
    path += ":.";
    setenv("PATH", path.c_str(), 1);
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
        if (sock == 0) {
            std::cout << get_current_dir() << "$ ";
            getline(std::cin, input);
        } else {
            std::cout << get_current_dir() << "$ " << std::flush;
            char buf[1024];
            int res = recv(sock, buf, sizeof(buf), 0);
            buf[res] = '\0';
            input = buf;
        }
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
            err = assign_cmd(program, args);
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
                std::vector<std::string> new_args = std::vector<std::string>{program};
                args.insert(args.begin(), new_args.begin(), new_args.end());
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

int main(int argc, char **argv) {
    po::options_description opt("Options");
    int port;
    opt.add_options()
            ("server", "Run as server")
            ("port",  po::value<int>(&port), "Port number");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, opt), vm);
    po::notify(vm);
    if (vm.count("server")) {
        struct sockaddr_in server;
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Can not open socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        memset(&server, 0, sizeof(server));

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(port);
        
        int binded = bind(sock, (struct sockaddr *) &server, sizeof(server));
        if (binded < 0) {
            std::cerr << "Can not bind" << std::endl;
            exit(EXIT_FAILURE);
        }
        listen(sock, 1);
        for (;;) {
            int ac_sock = accept(sock, NULL, NULL);
            if (ac_sock < 0){
                std::cerr << "Failed to accept connection" << std::endl;
                exit(1);
            }
            pid_t pid = fork();
            if (pid == -1) {
                std::cerr << "Failed to fork()" << std::endl;
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                close(ac_sock);
            } else {
                dup2(ac_sock, STDOUT_FILENO);  
                dup2(ac_sock, STDERR_FILENO); 
                process_execute(argc, argv, ac_sock);
                close(ac_sock);
            }
        }
    } else {
        process_execute(argc, argv);
    }
    return 0;
}

