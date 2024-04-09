#ifndef ERROR_HPP_
#define ERROR_HPP_

#include <iomanip>
#include <iostream>
#include <exception>

class MITRAXOXTException : public std::exception {
    public:
        MITRAXOXTException(std::string m) {
            this->_msg = m;
        };
        std::string what(void) {
            return this->_msg;
        };
    private:
        std::string _msg;
};

#endif /* !ERROR_HPP_ */