#ifndef _SERVER_TYPES_HPP_
#define _SERVER_TYPES_HPP_

#include <functional>
#include <cstdint>

typedef int8_t byte_t;
typedef uint8_t ubyte_t;

typedef std::function<ssize_t(const byte_t*)> ResponseWriter;

const char PATH_SEPARATOR = '/';

#endif