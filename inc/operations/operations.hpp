#ifndef ADDER_OPERATIONS_HPP
#include <iostream>
#include <vector>
#define ADDER_OPERATIONS_HPP

namespace operations {
    int merrno(std::vector<std::string>,bool);
    int mecho(std::vector<std::string>,bool);
    int mexport(std::vector<std::string>,bool);
    int mexit(std::vector<std::string>,bool);
    int mpwd(std::vector<std::string>,bool);
    int mcd(std::vector<std::string>,bool);
    int myscript(const std::vector<std::string>&, bool=1);
    int mycat(std::vector<std::string>, int);

}

#endif //ADDER_OPERATIONS_HPP
