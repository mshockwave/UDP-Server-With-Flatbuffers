#ifndef _SERVER_TYPES_HPP_
#define _SERVER_TYPES_HPP_

#include <string>
#include <functional>
#include <cstdint>

#include <boost/property_tree/ptree.hpp>

#include <schemas/types_generated.h>
#include "Session.hpp"

typedef uint8_t byte_t;
typedef int8_t sbyte_t;

#ifndef RECV_BUFFER_SIZE
#define RECV_BUFFER_SIZE (2 * (1 << 10)) //2KB
#endif

typedef std::function<ssize_t(const byte_t*, size_t)> ResponseWriter;

inline void SendStatusResponse(fbs::Status status, const ResponseWriter& resp_writer){
    flatbuffers::FlatBufferBuilder builder;
    auto general_resp = fbs::CreateGeneralResponse(builder, 0, status);
    fbs::FinishGeneralResponseBuffer(builder, general_resp);
    
    resp_writer(builder.GetBufferPointer(), builder.GetSize());
}

#define PATH_SEPARATOR '/'

#define ACCOUNT_FILE_NAME "accounts.json"

inline boost::property_tree::path GetPath(const std::string &path){
    return boost::property_tree::path(path, PATH_SEPARATOR);
}

#define PROFILE_PASSWORD_KEY    "password"
#define PROFILE_NIKNAME_KEY     "nickname"

#endif