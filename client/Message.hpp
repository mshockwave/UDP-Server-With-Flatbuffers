#ifndef _CLIENT_MESSAGE_HPP_
#define _CLIENT_MESSAGE_HPP_

#include <iostream>

#include <Utils.hpp>
#include <schemas/message.h>

#include "Context.hpp"

#ifndef FILE_CHUNK_SIZE
#define FILE_CHUNK_SIZE 1024 // 1KB
#endif

namespace msg{
    
    void InitScreens();
    
} //namespace msg

#endif