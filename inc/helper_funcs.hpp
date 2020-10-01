#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <unistd.h>

std::vector<std::string> tokenize(const std::string& input, std::string delim=" ")
{
    std::vector<std::string> result;
    typedef boost::char_separator<char> separator;
    //std::string delim = " ";
    boost::tokenizer<separator> tokens(input, separator(delim.c_str()));
    std::copy(tokens.begin(), tokens.end(), std::back_inserter(result));
    return result;
}

std::string get_current_dir() {
   char buff[FILENAME_MAX]; 
   getcwd( buff, FILENAME_MAX);
   std::string current_working_dir(buff);
   return current_working_dir;
}

void parse_args(std::string input, std::string& program, std::vector<std::string>& args, bool& help) {
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

    //std::string input = "mexport a=abc";

    po::variables_map vm;
    po::store(po::command_line_parser(tokenize(input))
                .options(all).positional(p).run(), vm);
    po::notify(vm);

    //bool help = 0;
    if (vm.count("help")) {
        help = 1;
    }
}