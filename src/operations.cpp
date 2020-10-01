#include "operations/operations.hpp"
#include <boost/tokenizer.hpp>
#include "helper_funcs.hpp"
extern char **environ;


int operations::merrno(std::vector<std::string> args, bool help) {
    return 0;
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
	if (args.size() > 1) {
		std::cerr << "Pass only one argument var_name=VAL(without spaces around \"=\")" << std::endl;
		return -1;
	}
	if (args[0].find("=") == std::string::npos){
		std::cerr << "Invalid input. Usage: var_name=VAL" << std::endl;
		return -2;
	}
	std::vector<std::string> tokenized = tokenize(args[0], "=");
    if ((tokenized.size() > 2) || args[0][0] == '=') {
    	std::cerr << "Invalid input. Usage: var_name=VAL" << std::endl;
    	return -2;
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

