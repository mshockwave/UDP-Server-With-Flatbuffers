#include "Session.hpp"

#include <unordered_map>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

typedef std::unordered_map<session::key_type, std::string> value_map_type;
static std::unordered_map< std::string ,value_map_type > SessionValueMap;
typedef std::unordered_map< std::string ,value_map_type >::iterator session_value_map_it_type;

namespace session {
    
    const char* SESSION_KEY_USERNAME = "username";
    
    fbs_offset_type NewSession(flatbuffers::FlatBufferBuilder& builder,
                               const std::string &username){
        
        boost::uuids::random_generator hash_gen;
        auto hash_str = boost::uuids::to_string(hash_gen());
        auto new_session = fbs::CreateSession(builder,
                                              /*Token*/
                                              builder.CreateString(hash_str));
        value_map_type value_map;
        value_map[SESSION_KEY_USERNAME] = username;
        SessionValueMap[hash_str] = value_map;
        return new_session;
    }
    
    std::string GetStringValue(const fbs_type &fbs_session, const key_type &key){
        auto token_str = fbs_session.token()->str();
        
        session_value_map_it_type it_result;
        if( (it_result = SessionValueMap.find(token_str)) != SessionValueMap.end()){
            auto& value_map = it_result->second;
            return value_map[key];
        }else{
            throw BadTransformException("Key not found");
        }
    }
    void PutStringValue(const fbs_type &fbs_session, const key_type &key, std::string &v){
        auto token_str = fbs_session.token()->str();
        
        session_value_map_it_type it_result;
        if( (it_result = SessionValueMap.find(token_str)) != SessionValueMap.end()){
            auto& value_map = it_result->second;
            value_map[key] = v;
        }else{
            throw BadTransformException("Key not found");
        }
    }
    
    int GetIntValue(const fbs_type &fbs_session, const key_type &key){
        auto token_str = fbs_session.token()->str();
        
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
    void PutIntValue(const fbs_type &fbs_session, const key_type &key, int v){
        auto token_str = fbs_session.token()->str();
        
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
    
    long GetLongValue(const fbs_type &fbs_session, const key_type &key){
        auto token_str = fbs_session.token()->str();
        
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
    void PutLongValue(const fbs_type &fbs_session, const key_type &key, long v){
        auto token_str = fbs_session.token()->str();
        
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
    
} //namespace session