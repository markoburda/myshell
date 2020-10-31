#include "operations/operations.hpp"
#include <boost/tokenizer.hpp>
#include "helper_funcs.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <a.out.h>
#include <fstream>
extern char **environ;

int operations::myscript(const std::vector<std::string>& files, bool forking){
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
                    std::cerr << "Failed to exec()" << std::endl;
                    return EXIT_FAILURE;
                }
            }
        
        }
        if (!forking) {
	        std::vector<const char*> arg_for_c;
	        arg_for_c.push_back(nullptr);
	        execvp(filename.c_str(), const_cast<char* const*>(arg_for_c.data()));
	        std::cerr << "Failed to exec()" << std::endl;
	        return EXIT_FAILURE;
    	}
    }
    
    return EXIT_SUCCESS;
        
}


int operations::mpwd(std::vector<std::string> args, bool help) {
	if (help) {
		std::cout << "Print current directory\nUsage:\nmpwd [-h|--help] \n-h, --help\t Print this help message\n ";
		return 0;
	}
	if (args.size()!=0) {
		std::cerr << "Unsupported options given. See --help\n";
		return -1;
	}
	std::cout << get_current_dir() << std::endl;
    return 0;
}

int operations::mcd(std::vector<std::string> args, bool help) {
    if (help) {
		std::cout << "Cnahge current directory\nUsage:\nmpwd <path> [-h|--help] \n-h, --help\t Print this help message\n  path\t  New current directory\n";
		return 0;
	}
	if ((args.size() > 1) || (args.size() == 0)) {
		std::cerr << "Wrong number of arguments. See --help\n";
		return -1;
	}
	if (chdir(args[0].c_str())){
        std::cerr << "cd: " << args[0] << ": No such file or directory" << std::endl;
    	return -2;
    }
    return 0;

}

int operations::mecho (std::vector<std::string> args, bool help) {
	if (help) {
		std::cout << "Print given message or variable\nUsage:\nmecho [-h|--help] [text|$<var_name>] [text|$<var_name>]  [text|$<var_name>]\
		 ...\n-h, --help\t Print this help message\n text\t Print text\n $<var_name>\t Print environment variable\n";
		return 0;
	}

	for(auto& arg : args){
        if (arg[0] == '$')
        {
            if (getenv(arg.substr(1, arg.size()-1).c_str()) == nullptr){
                std::cerr << "Warning: there is no such variable <" << arg << ">\n";
                return EXIT_FAILURE;
            }
            else{
                std::cout << getenv(arg.substr(1, arg.size()-1).c_str()) << std::endl;
            }
        }
        else{
            std::cout << arg << std::endl;
        }

    }

    return 0;
}

int operations::mexport(std::vector<std::string> args, bool help) {
    if (help) {
		std::cout << "Make environment variable\nUsage:\nmexport [-h|--help] var_name=VAL \n -h, --help\t Print this help message\
		\n var_name\tName of environment variable\n VAL\t Value to be asigned to variable\n";
		return 0; 
	}
	if (args[0].find("=") == std::string::npos){
		std::cerr << "Invalid input. Usage: var_name=VAL" << std::endl;
		return -1;
	}
	std::string str_arg= "";
	for (size_t i = 0; i < args.size(); i++){
		if (i != 0)
			str_arg += " ";
		str_arg = str_arg + args[i];

	}
	std::vector<std::string> tokenized = tokenize(str_arg, "=");
    if ((tokenized.size() > 2) || args[0][0] == '=') {
    	std::cerr << "Invalid input. Usage: var_name=VAL" << std::endl;
    	return -1;
    }
    int res = 0;
    if (tokenized.size() == 1) {
    	res = setenv(tokenized[0].c_str(), "", 1);
    } else if (tokenized.size() == 2) {
    	res = setenv(tokenized[0].c_str(), tokenized[1].c_str(), 1);
    }
    if (res) {
    	std::cerr << "Could not set environment variable" << std::endl;
    	return -3;
    }
	return 0;
}

int operations::mexit(std::vector<std::string> args, bool help) {
    if (help) {
		std::cout << "Exit with given code(default 0)\nUsage:\nmexit [-h|--help] [exit_code] \n -h, --help\t Print this help message\
		\n exit_code Code to exit with\n";
		return 0;
	}
	if (args.size()==0){
		std::cout << "Exit with code 0" << std::endl;
		exit(0);
	}
	else {
		std::cout << "Exit with code " << args[0] << std::endl;
		exit(std::atoi(args[0].c_str()));
	}
}





