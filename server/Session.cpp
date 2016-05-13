#include "Session.hpp"
#include <Utils.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

static std::unordered_map< std::string ,session::value_map_type > SessionValueMap;
typedef std::unordered_map< std::string ,session::value_map_type >::iterator session_value_map_it_type;
static std::unordered_map<std::string, std::string> SessionHashUserMap;
static std::unordered_map<std::string, Request::SocketAddrIn> SessionClientAddrMap;

namespace session {
    
    const char* SESSION_KEY_USERNAME = "username";
    
    const char* SESSION_KEY_TIMESTAMP = "last-active-time";
    
    const char* SESSION_KEY_CLIENT_ADDR = "client-addr";
    
    const char* SESSION_KEY_CAN_CHAT = "can-chat";
    
    fbs_offset_type NewSession(flatbuffers::FlatBufferBuilder& builder,
                               const std::string &username,
                               const Request::SocketAddrIn& client_addr){
        
        boost::uuids::random_generator hash_gen;
        auto hash_str = boost::uuids::to_string(hash_gen());
        auto new_session = fbs::CreateSession(builder,
                                              /*Token*/
                                              builder.CreateString(hash_str));
        value_map_type value_map;
        value_map[SESSION_KEY_USERNAME] = username;
        value_map[SESSION_KEY_TIMESTAMP] = utils::GetCurrentTime();
        
        //Hash for client address
        auto addr_hash_str = boost::uuids::to_string(hash_gen());
        SessionClientAddrMap[addr_hash_str] = client_addr;
        value_map[SESSION_KEY_CLIENT_ADDR] = addr_hash_str;
        
        SessionValueMap[hash_str] = value_map;
        SessionHashUserMap[username] = hash_str;
        return new_session;
    }
    
    std::string GetHashByUsername(const std::string& username){
        auto it_result = SessionHashUserMap.find(username);
        if(it_result == SessionHashUserMap.end()){
            throw BadTransformException("Username not found");
        }
        return it_result->second;
    }
    
    bool IsSessionExist(const fbs_type &fbs_session){
        auto token_str = fbs_session.token()->str();
        return (SessionValueMap.find(token_str) != SessionValueMap.end());
    }
    
    void RemoveSession(const fbs_type &fbs_session){
        auto token_str = fbs_session.token()->str();
        
        session_value_map_it_type it_result;
        if( (it_result = SessionValueMap.find(token_str)) != SessionValueMap.end()){
            SessionValueMap.erase(it_result);
        }else{
            throw BadTransformException("Key not found");
        }
    }
    
    const Request::SocketAddrIn& GetClientAddr(const std::string &token_str){
        auto addr_token = GetStringValue(token_str, SESSION_KEY_CLIENT_ADDR);
        auto it_result = SessionClientAddrMap.find(addr_token);
        if(it_result != SessionClientAddrMap.end()){
            return it_result->second;
        }else{
            throw BadTransformException("Address Token String Not exist");
        }
    }
    const Request::SocketAddrIn& GetClientAddr(const fbs_type &fbs_session){
        auto token_str = fbs_session.token()->str();
        return GetClientAddr(token_str);
    }
    
    std::string GetStringValue(const std::string& token_str, const key_type &key){
        
        session_value_map_it_type it_result;
        if( (it_result = SessionValueMap.find(token_str)) != SessionValueMap.end()){
            auto& value_map = it_result->second;
            return value_map[key];
        }else{
            throw BadTransformException("Key not found");
        }
    }
    std::string GetStringValue(const fbs_type &fbs_session, const key_type &key){
        auto token_str = fbs_session.token()->str();
        return GetStringValue(token_str, key);
    }
    void PutStringValue(const std::string& token_str, const key_type &key, std::string &v){
        
        session_value_map_it_type it_result;
        if( (it_result = SessionValueMap.find(token_str)) != SessionValueMap.end()){
            auto& value_map = it_result->second;
            value_map[key] = v;
        }else{
            throw BadTransformException("Key not found");
        }
    }
    void PutStringValue(const fbs_type &fbs_session, const key_type &key, std::string &v){
        auto token_str = fbs_session.token()->str();
        PutStringValue(token_str, key, v);
    }
    
    int GetIntValue(const std::string &token_str, const key_type &key){
        session_value_map_it_type it_result;
        if( (it_result = SessionValueMap.find(token_str)) != SessionValueMap.end()){
            auto& value_map = it_result->second;
            auto value_str = value_map[key];
            try{
                return std::stoi(value_str);
            }catch(...){
                throw BadTransformException("Conversion failed");
            }
        }else{
            throw BadTransformException("Key not found");
        }
    }
    int GetIntValue(const fbs_type &fbs_session, const key_type &key){
        auto token_str = fbs_session.token()->str();
        return GetIntValue(token_str, key);
    }
    void PutIntValue(const std::string &token_str, const key_type &key, int v){
        
        session_value_map_it_type it_result;
        if( (it_result = SessionValueMap.find(token_str)) != SessionValueMap.end()){
            auto& value_map = it_result->second;
            try{
                value_map[key] = std::to_string(v);
            }catch(...){
                throw BadTransformException("Conversion failed");
            }
        }else{
            throw BadTransformException("Key not found");
        }
    }
    void PutIntValue(const fbs_type &fbs_session, const key_type &key, int v){
        auto token_str = fbs_session.token()->str();
        PutIntValue(token_str, key, v);
    }
    
    long GetLongValue(const std::string &token_str, const key_type &key){
        
        session_value_map_it_type it_result;
        if( (it_result = SessionValueMap.find(token_str)) != SessionValueMap.end()){
            auto& value_map = it_result->second;
            auto value_str = value_map[key];
            try{
                return std::stol(value_str);
            }catch(...){
                throw BadTransformException("Conversion failed");
            }
        }else{
            throw BadTransformException("Key not found");
        }
    }
    long GetLongValue(const fbs_type &fbs_session, const key_type &key){
        auto token_str = fbs_session.token()->str();
        return GetLongValue(token_str, key);
    }
    void PutLongValue(const std::string &token_str, const key_type &key, long v){
        
        session_value_map_it_type it_result;
        if( (it_result = SessionValueMap.find(token_str)) != SessionValueMap.end()){
            auto& value_map = it_result->second;
            try{
                value_map[key] = std::to_string(v);
            }catch(...){
                throw BadTransformException("Conversion failed");
            }
        }else{
            throw BadTransformException("Key not found");
        }
    }
    void PutLongValue(const fbs_type &fbs_session, const key_type &key, long v){
        auto token_str = fbs_session.token()->str();
        PutLongValue(token_str, key, v);
    }
    
} //namespace session