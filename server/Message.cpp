#include "Message.hpp"

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <memory>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid//uuid_io.hpp>

#include <schemas/message.h>
#include <Utils.hpp>
#include "Types.hpp"
#include "Session.hpp"

/*
 * A channel can be a person or a group
 */
typedef std::string channel_id_t;
/*Channel id -> Group member account names*/
static std::unordered_map< channel_id_t, std::unordered_set<std::string> > ChannelMemberMap;
/*Sender name -> message content*/
typedef struct MsgEntity{
    std::string Sender;
    
    fbs::msg::MsgContent ContentType;
    //String Content
    std::string String;
    
    //File Chunk Content
    long Sequence;
    std::string FileName;
    std::shared_ptr<sbyte_t> FileChunk;
    size_t ChunkSize;
    
    MsgEntity() :
    Sender(""),
    ContentType(fbs::msg::MsgContent_NONE),
    String(""),
    Sequence(0),
    FileName(""),
    //FileChunk(nullptr),
    ChunkSize(0){}
    
} msg_entity_t;
/*Channel id -> message queue*/
typedef std::unordered_map< channel_id_t, std::vector<msg_entity_t> > mailbox_t;
static std::unordered_map<channel_id_t, fbs::msg::ChannelType> ChannelTypeMap;
/*User account name -> His mail boxes*/
static std::unordered_map<std::string, mailbox_t> UserMailBoxes;

namespace handlers{
    
    namespace message{
        
        const HandleFunc handleCreateChannel = HANDLE_FUNC(){
            
            const auto send_status = [&response_writer](fbs::Status status)->void{
                flatbuffers::FlatBufferBuilder builder;
                auto resp = fbs::msg::CreateCreateChannelResponse(builder,
                                                                  status);
                fbs::msg::FinishCreateChannelResponseBuffer(builder, resp);
                response_writer(builder.GetBufferPointer(), builder.GetSize());
            };
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::msg::VerifyCreateChannelBuffer(verifier)){
                
                auto* req = fbs::msg::GetCreateChannel(request.payload()->Data());
                const auto* session = req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        
                        std::string channel_id("");
                        if(req->channel_type() == fbs::msg::ChannelType::ChannelType_PRIVATE){
                            
                            //Create channel id
                            boost::uuids::random_generator hash_gen;
                            channel_id = boost::uuids::to_string(hash_gen());
                            
                        }else{ //group
                            channel_id = req->name()->str();
                            if(channel_id.length() == 0){
                                SendStatusResponse(fbs::Status_INVALID_REQUEST_ARGUMENT, response_writer);
                                return;
                            }
                            
                            //Group name need to be unique
                            if(ChannelMemberMap.find(channel_id) != ChannelMemberMap.end()){
                                //Had Exist
                                SendStatusResponse(fbs::Status_INVALID_REQUEST_ARGUMENT, response_writer);
                                return;
                            }
                            
                        }
                        
                        ChannelTypeMap[channel_id] = req->channel_type();
                        
                        for(auto member : *(req->member())){
                            ChannelMemberMap[channel_id].insert(member->str());
                        }
                        ChannelMemberMap[channel_id].insert(username);
                        
                        flatbuffers::FlatBufferBuilder builder;
                        auto resp = fbs::msg::CreateCreateChannelResponse(builder,
                                                                          fbs::Status_OK, 0,
                                                                          builder.CreateString(channel_id));
                        fbs::msg::FinishCreateChannelResponseBuffer(builder, resp);
                        
                        response_writer(builder.GetBufferPointer(), builder.GetSize());
                        
                    }catch(const session::BadTransformException&){
                        send_status(fbs::Status_AUTH_ERROR);
                    }
                    
                }else{
                    send_status(fbs::Status_AUTH_ERROR);
                }
                
            }else{
                Log::W("Create Channel Handler") << "Payload format invalid" << std::endl;
                send_status(fbs::Status_PAYLOAD_FORMAT_INVALID);
            }
        };
        const HandleFunc handleViewChannels = HANDLE_FUNC(){
            const auto send_status = [&response_writer](fbs::Status status)->void{
                flatbuffers::FlatBufferBuilder builder;
                auto resp = fbs::msg::CreateViewChannelResponse(builder,
                                                                status);
                fbs::msg::FinishViewChannelResponseBuffer(builder, resp);
                response_writer(builder.GetBufferPointer(), builder.GetSize());
            };
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::msg::VerifyViewChannelRequestBuffer(verifier)){
                
                auto* req = fbs::msg::GetViewChannelRequest(request.payload()->Data());
                const auto* session = req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        
                        flatbuffers::FlatBufferBuilder builder;
                        std::vector< flatbuffers::Offset<fbs::msg::ChannelEntity> > results;
                        
                        //Get channel names that contains that user
                        for(auto& channel : ChannelMemberMap){
                            if(channel.second.find(username) == channel.second.end()){
                                continue;
                            }
                            
                            auto channel_id_fbs = builder.CreateString(channel.first);
                            auto type = ChannelTypeMap[channel.first];
                            if(type == fbs::msg::ChannelType::ChannelType_PRIVATE){
                                //Get another one
                                for(auto& it_user : channel.second){
                                    if(it_user != username){
                                        auto display_name = builder.CreateString(it_user);
                                        auto ch = fbs::msg::CreateChannelEntity(builder,
                                                                                type,
                                                                                channel_id_fbs,
                                                                                display_name);
                                        results.push_back(ch);
                                        break;
                                    }
                                }
                            }else{ //group
                                auto ch = fbs::msg::CreateChannelEntity(builder,
                                                                        type,
                                                                        channel_id_fbs,
                                                                        channel_id_fbs);
                                results.push_back(ch);
                            }
                        }
                        
                        auto resp = fbs::msg::CreateViewChannelResponse(builder,
                                                                        fbs::Status_OK,0,
                                                                        builder.CreateVector(results));
                        fbs::msg::FinishViewChannelResponseBuffer(builder, resp);
                        
                        response_writer(builder.GetBufferPointer(), builder.GetSize());
                        
                    }catch(const session::BadTransformException&){
                        send_status(fbs::Status_AUTH_ERROR);
                    }
                    
                }else{
                    send_status(fbs::Status_AUTH_ERROR);
                }
                
            }else{
                Log::W("View Channel Handler") << "Payload Format Error" << std::endl;
                send_status(fbs::Status_PAYLOAD_FORMAT_INVALID);
            }
        };
        
        const HandleFunc handleJoinGroup = HANDLE_FUNC(){
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::msg::VerifyJoinGroupRequestBuffer(verifier)){
                
                auto* req = fbs::msg::GetJoinGroupRequest(request.payload()->Data());
                const auto* session = req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    try{
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        const auto& group_name = req->group_name()->str();
                        
                        if(ChannelMemberMap.find(group_name) == ChannelMemberMap.end()){
                            //Not found
                            SendStatusResponse(fbs::Status_INVALID_REQUEST_ARGUMENT, response_writer);
                        }else{
                            ChannelMemberMap[group_name].insert(username);
                            SendStatusResponse(fbs::Status_OK, response_writer);
                        }
                        
                    }catch(const session::BadTransformException&){
                        SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                    }
                    
                }else{
                    SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                }
                
            }else{
                Log::W("Join Group Handler") << "Payload Format Error" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
        
        const HandleFunc handleLeaveGroup = HANDLE_FUNC(){
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::msg::VerifyLeaveGroupRequestBuffer(verifier)){
                
                auto* req = fbs::msg::GetLeaveGroupRequest(request.payload()->Data());
                const auto* session = req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    try{
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        const auto& group_name = req->group_name()->str();
                        
                        if(ChannelMemberMap.find(group_name) == ChannelMemberMap.end()){
                            //Not found
                            SendStatusResponse(fbs::Status_INVALID_REQUEST_ARGUMENT, response_writer);
                        }else{
                            ChannelMemberMap[group_name].erase(username);
                            SendStatusResponse(fbs::Status_OK, response_writer);
                        }
                        
                    }catch(const session::BadTransformException&){
                        SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                    }
                    
                }else{
                    SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                }
                
            }else{
                Log::W("Join Group Handler") << "Payload Format Error" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
        
        const HandleFunc handleGetMessage = HANDLE_FUNC(){
            const auto send_status = [&response_writer](fbs::Status status)->void{
                flatbuffers::FlatBufferBuilder builder;
                auto resp = fbs::msg::CreateGetMsgResponse(builder,
                                                           0,
                                                           status);
                fbs::msg::FinishGetMsgResponseBuffer(builder, resp);
                response_writer(builder.GetBufferPointer(), builder.GetSize());
            };
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::msg::VerifyGetMsgRequestBuffer(verifier)){
                
                auto* req= fbs::msg::GetGetMsgRequest(request.payload()->Data());
                const auto* session = req->session();
                
                if(session::IsSessionExist(*session)){
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        const auto& channel_id = req->channel()->str();
                        
                        auto& mailbox = UserMailBoxes[username];
                        auto& msg_queue = mailbox[channel_id];
                        
                        flatbuffers::FlatBufferBuilder builder;
                        std::vector<  flatbuffers::Offset<fbs::msg::MsgEntity> > ret_list;
                        
                        if(!msg_queue.empty()){
                            /*
                             Do not fetch all the message at a time
                             Since the total vector content size 
                             May exceed flatbuffers offset range
                             */
                            const auto& msg_item = *(msg_queue.begin());
                            const auto& sender = msg_item.Sender;
                            auto msg_entity = fbs::msg::CreateMsgEntity(builder);
                            switch(msg_item.ContentType){
                                case fbs::msg::MsgContent_StringContent:{
                                    auto str_offset = builder.CreateString(msg_item.String);
                                    auto string_content = fbs::msg::CreateStringContent(builder,
                                                                                        str_offset);
                                    msg_entity = fbs::msg::CreateMsgEntity(builder,
                                                                           builder.CreateString(sender),
                                                                           msg_item.ContentType,
                                                                           string_content.Union());
                                    break;
                                }
                                    
                                case fbs::msg::MsgContent_FileChunk:{
                                    auto file_name_offset = builder.CreateString(msg_item.FileName);
                                    auto file_bin_offset = builder.CreateVector(msg_item.FileChunk.get(),
                                                                                msg_item.ChunkSize);
                                    auto file_content = fbs::msg::CreateFileChunk(builder,
                                                                                  file_name_offset,
                                                                                  msg_item.Sequence,
                                                                                  file_bin_offset);
                                    msg_entity = fbs::msg::CreateMsgEntity(builder,
                                                                           builder.CreateString(sender),
                                                                           msg_item.ContentType,
                                                                           file_content.Union());
                                    break;
                                }
                                    
                                case fbs::msg::MsgContent_NONE:{
                                    msg_entity = fbs::msg::CreateMsgEntity(builder,
                                                                           builder.CreateString(sender),
                                                                           msg_item.ContentType);
                                    break;
                                }
                            }
                            
                            ret_list.push_back(msg_entity);
                            msg_queue.erase(msg_queue.begin());
                        }
                        
                        auto resp = fbs::msg::CreateGetMsgResponse(builder,
                                                                   builder.CreateVector(ret_list),
                                                                   fbs::Status_OK, 0);
                        fbs::msg::FinishGetMsgResponseBuffer(builder, resp);
                        
                        response_writer(builder.GetBufferPointer(), builder.GetSize());
                        
                    }catch(const session::BadTransformException&){
                        send_status(fbs::Status_AUTH_ERROR);
                    }
                }else{
                    send_status(fbs::Status_AUTH_ERROR);
                }
                
            }else{
                Log::W("Get Message Handler") << "Payload Format Error" << std::endl;
                send_status(fbs::Status_PAYLOAD_FORMAT_INVALID);
            }
        };
        
        const HandleFunc handlePutMessage = HANDLE_FUNC(){
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::msg::VerifyPutMsgRequestBuffer(verifier)){
                
                auto* req = fbs::msg::GetPutMsgRequest(request.payload()->Data());
                const auto* session = req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        const auto& channel_id = req->channel_id()->str();
                        
                        msg_entity_t msg_entity;
                        const auto* msg_content = req->msg();
                        msg_entity.ContentType = msg_content->content_type();
                        msg_entity.Sender = username;
                        
                        switch(msg_content->content_type()){
                            case fbs::msg::MsgContent_StringContent:{
                                const auto* str_content =
                                    reinterpret_cast<const fbs::msg::StringContent*>(msg_content->content());
                                
                                msg_entity.String = str_content->data()->str();
                                break;
                            }
                                
                            case fbs::msg::MsgContent_FileChunk:{
                                const auto* file_content =
                                    reinterpret_cast<const fbs::msg::FileChunk*>(msg_content->content());
                                
                                msg_entity.Sequence = file_content->seq();
                                msg_entity.FileName = file_content->file_name()->str();
                                if(file_content->seq() > 0){
                                    const auto* file_data = file_content->data();
                                    auto* storage = new sbyte_t[file_data->size()];
                                    ::memcpy(storage, file_data->Data(), file_data->size());
                                    msg_entity.ChunkSize = file_data->size();
                                    msg_entity.FileChunk.reset(storage);
                                }else{
                                    msg_entity.ChunkSize = 0;
                                    msg_entity.FileChunk.reset();
                                }
                                
                                break;
                            }
                                
                            case fbs::msg::MsgContent_NONE:{
                                break;
                            }
                        }
                        
                        //Send to channel's member(s)
                        const auto& member_set = ChannelMemberMap[channel_id];
                        for(const auto& member : member_set){
                            if(member == msg_entity.Sender) continue;
                            auto& mail_box = UserMailBoxes[member];
                            mail_box[channel_id].push_back(msg_entity);
                        }
                        
                        SendStatusResponse(fbs::Status_OK, response_writer);
                        
                    }catch(const session::BadTransformException&){
                        SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                    }
                    
                }else{
                   SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                }
                
            }else{
                Log::W("Get Message Handler") << "Payload Format Error" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
    }
    
    void InitMessageHandlers(Router &router){
        router.Path("/channel/new", message::handleCreateChannel)
        .Path("/channel", message::handleViewChannels)
        .Path("/group/join", message::handleJoinGroup)
        .Path("/group/leave", message::handleLeaveGroup)
        .Path("/get", message::handleGetMessage)
        .Path("/put", message::handlePutMessage);
    }
    
} //namespace handlers