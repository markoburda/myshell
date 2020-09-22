#include <iostream>
#include <boost/program_options.hpp>
#include "operations/operations.hpp"
#include <vector>
#include <string>


std::string right_path(const std::vector<std::string>& paths, const std::string& name){
    for (const auto& i: paths){
        if (access ((i + name).c_str(), F_OK) == 0){
            return i + name;
        }
    }
    return "";
}
int main(int argc, char **argv) {
    int variable_a, variable_b;
    std::vector<std::string> paths;
    paths.emplace_back("");
    paths.emplace_back("./bin/ls/");
    std::string a = "mycat";
    std::string res = right_path(paths, a);
    if(!res.empty()){
        std::cout << res << '\n';
    }
    else{
        std::cerr << "Wrong path\n";
    }
    res = right_path(paths, "wrong");
    std::cin >> a;
    if(!res.empty()){
        std::cout << res << '\n';
    }
    else{
        std::cerr << "Wrong path\n";
    }
    std::cout << res;
    return EXIT_SUCCESS;
}
