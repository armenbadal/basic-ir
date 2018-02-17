
#ifndef ERRORS_HXX
#define ERRORS_HXX

#include <exception>
#include <string>

// TODO: վերանայել այս դասերը

namespace basic {
//
class ParseError : exception {
private:
    string message = "";

public:
    ParseError(const string& mes)
	  : message(mes)
    {
    }
    const char* what() const noexcept
    {
        return message.c_str();
    }
};

//
class TypeError : exception {
private:
    string message = "";

public:
    TypeError(const string& mes)
	  : message(mes)
    {
    }
    const char* what() const noexcept
    {
        return message.c_str();
    }
};
} // basic

#endif // ERRORS_HXX

