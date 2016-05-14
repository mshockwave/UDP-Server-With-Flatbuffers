#include "Message.hpp"

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>

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
typedef std::pair<std::string,fbs::msg::MsgContent> msg_entity_t;
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
                send_status(fbs::Status_PAYLOAD_FORMAT_INVALID);
            }
        };
        
        const HandleFunc handleGetMessage = HANDLE_FUNC(){
            
        };
        
        const HandleFunc handlePutMessage = HANDLE_FUNC(){
            
        };
    }
    
    void InitMessageHandlers(Router &router){
        router.Path("/channel/new", message::handleCreateChannel)
        .Path("/channel", message::handleViewChannels)
        .Path("/get", message::handleGetMessage)
        .Path("/put", message::handlePutMessage);
    }
    
} //namespace handlers