#ifndef CALLEE_H_HELLO_23456778
#define CALLEE_H_HELLO_23456778

#include <string>


struct callee
{
    
    callee(const callee &) = delete;
    callee(const callee &&) = delete;

    static callee &get_instance();

    bool is_valid();

    const std::string & get_callee();
    const unsigned short get_port();

    ~callee() = default;

private:
   callee();
 

private:
    std::string callee_server;
    unsigned short port;
};

#endif