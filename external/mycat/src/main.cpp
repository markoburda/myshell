#include <iostream>
#include <boost/program_options.hpp>

#include "operations/operations.hpp"

int main(int argc, char **argv) {
    std::vector<std::string> files;
    int hex = 0;

    namespace po = boost::program_options;

    po::options_description visible("Supported options");
    visible.add_options()
            ("help,h", "Print this help message.")
            ("A,A", "Display invisible symbols in hexadecimal  format");

    po::options_description hidden("Hidden options");
    hidden.add_options()
            ("opt", po::value<std::vector<std::string>>(&files)->multitoken(), "description");


    po::positional_options_description p;
    p.add("opt", -1);

    po::options_description all("All options");
    all.add(visible).add(hidden);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage:\n  mycat [-h|--help] [-A] <file1> <file2> ... <fileN>\n" << visible << std::endl;
        return EXIT_SUCCESS;
    }
    if (vm.count("A")) {
       hex++; 
    }
    int err = operations::mycat(files, hex);
    if (err) {
        return -1;
    }
    return EXIT_SUCCESS;
}
