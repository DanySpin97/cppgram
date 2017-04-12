#ifndef __CPPGRAM_EXCEPTIONS_HPP
#define __CPPGRAM_EXCEPTIONS_HPP

#include <exception>

namespace cppgram
{
/**
 * \class ThreadException
 * \inherits std::exception
 * \brief raises if thread could not start (*fatal*).
 */
class ThreadException : public std::exception
{
    public:
    virtual const char *what() const throw() { return "Thread could not started correctly"; }
};

/**
 * \class JsonParseError
 * \inherits std::exception
 * \brief If jsoncpp can't parse the JSON document (*critical*).
 */
class JsonParseError : public std::exception
{
    public:
    virtual const char *what() const throw() { return "Error while parsing JSON document"; }
};

/**
 * \class InlineKeyboardNotValid
 * \inherits std::exception
 * \brief raises if InlineKeyboard is not valid (*critical*).
 */
class InlineKeyboardNotValid : public std::exception
{
    public:
    virtual const char *what() const throw() { return "Inline keyboard contains not valid data"; }
};

/**
 * \class MessageNotCreated
 * \inherits std::exception
 * \brief Could not create message.
 * \description Could not create message to return for api methods.
 */
class MessageNotCreated : public std::exception
{
    public:
    virtual const char *what() const throw()
    {
        return "Could not create message to return for api methods.";
    }
};
}

#endif
