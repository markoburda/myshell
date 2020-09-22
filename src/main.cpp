#include <iostream>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <string>
#include "operations/operations.hpp"


std::vector<std::string> tokenize(const std::string& input)
{
    std::vector<std::string> result;
    typedef boost::char_separator<char> separator;
    std::string delim = " ";
    boost::tokenizer<separator> tokens(input, separator(delim.c_str()));
    std::copy(tokens.begin(), tokens.end(), std::back_inserter(result));
    return result;
}



int main(int argc, char **argv) {

    std::string program;
    std::vector<std::string> args;

    namespace po = boost::program_options;
    po::options_description visible("Options");
    visible.add_options()
       ("help,h", "Print this help message.");

    po::options_description hidden("Hidden options");
    hidden.add_options()
            ("program", po::value<std::string>(&program))
            ("arguments", po::value<std::vector<std::string>>(&args)->multitoken());

    po::positional_options_description p;
    p.add("program", 1);
    p.add("arguments", -1);   


    po::options_description all("All options");
    all.add(visible).add(hidden);

    std::string input = "merrno -h a b c";

    po::variables_map vm;
    po::store(po::command_line_parser(tokenize(input))
                .options(all).positional(p).run(), vm);
    po::notify(vm);
    std::cout << program << std::endl;
    for (auto& arg : args){
        std::cout << arg << std::endl;
    }
    if (vm.count("help")) {
        std::cout << "Help...\n" << visible << std::endl;
        return EXIT_SUCCESS;
    }
    
    return EXIT_SUCCESS;
}



