#ifndef _MRPC_EXCEPTION_H_
#define _MRPC_EXCEPTION_H_

#include <exception>

namespace MRPC {
    class MRPCError : public std::exception {
    public:
        std::string message;
        MRPCError() { message = ""; }
        MRPCError(std::string message) { this->message = message; }
        const char * what () const throw ()
        {
            return message.c_str();
        }
    };
    class NoReturn : public MRPCError { using MRPCError::MRPCError; };
    class InvalidPath : public MRPCError { using MRPCError::MRPCError; };
}

#endif