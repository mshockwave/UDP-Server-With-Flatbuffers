#include "Message.hpp"
#include <Log.hpp>
#include <Utils.hpp>

#include <string>
#include <vector>

extern "C"{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
}

#include <boost/tokenizer.hpp>

namespace msg {
    
    const context::ScreenHandler handleMsgEntry = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::MAIN;
        
        std::cout << "Create [G]roup Chat\t" << "Create [P]rivate Chat" << std::endl;
        std::cout << "View [C]onversations" << std::endl;
        std::cout << "[J]oin Group Chat" << std::endl;
        std::cout << "[B]ack" << std::endl;
        std::cout << context::PROMPT_CHAR;
        
        char input_char;
        std::cin >> input_char;
        switch(std::tolower(input_char)){
            case 'g':{
                next_screen = context::Screen::MSG_CREATE_GROUP_CHANNEL;
                break;
            }
                
            case 'p':{
                next_screen = context::Screen::MSG_CREATE_PRIVATE_CHANNEL;
                break;
            }
                
            case 'b':{
                next_screen = context::Screen::MAIN;
                break;
            }
                
            case 'c':{
                next_screen = context::Screen::MSG_VIEW_CHANNELS;
                break;
            }
                
            case 'j':{
                next_screen = context::Screen::MSG_JOIN_GROUP;
                break;
            }
                
            default:
                std::cout << "Unrecognized command: " << input_char << std::endl;
                next_screen = context::Screen::STAY;
        }
        
        return next_screen;
    };
    
    const context::ScreenHandler handleCreateChannel = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::MSG_ENTRY;
        
        flatbuffers::FlatBufferBuilder builder_msg, builder_req;
        std::vector< flatbuffers::Offset<flatbuffers::String> > members;
        flatbuffers::Offset<fbs::msg::CreateChannel> req;
        if(current_screen == context::Screen::MSG_CREATE_PRIVATE_CHANNEL){
            
            std::cout << "Enter account: ";
            std::string account_name("");
            std::cin >> account_name;
            
            members.push_back(builder_msg.CreateString(account_name));
            auto session = fbs::CreateSession(builder_msg,
                                              builder_msg.CreateString(context::CurrentTokenStr));
            req = fbs::msg::CreateCreateChannel(builder_msg, session,
                                                builder_msg.CreateString(""),
                                                fbs::msg::ChannelType_PRIVATE,
                                                builder_msg.CreateVector(members));
            
        }else if(current_screen == context::Screen::MSG_CREATE_GROUP_CHANNEL){
            
            std::cout << "Enter group name: ";
            std::string group_name("");
            std::cin >> group_name;
            
            if(group_name.length() == 0){
                std::cout << "Error: Group name can't be empty" << std::endl;
                return context::Screen::STAY;
            }
            
            std::cout << "Enter members(separated by comma): " << std::endl;
            std::cin.ignore();
            std::string member_str("");
            std::getline(std::cin, member_str);
            
            utils::TrimString(member_str, ',');
            boost::char_separator<char> separator(", ");
            boost::tokenizer< boost::char_separator<char> > tokens(member_str, separator);
            for(auto token : tokens){
                members.push_back(builder_msg.CreateString(token));
            }
            
            auto session = fbs::CreateSession(builder_msg,
                                              builder_msg.CreateString(context::CurrentTokenStr));
            req = fbs::msg::CreateCreateChannel(builder_msg, session,
                                                builder_msg.CreateString(group_name),
                                                fbs::msg::ChannelType_GROUP,
                                                builder_msg.CreateVector(members));
        }else{
            return next_screen;
        }
        
        fbs::msg::FinishCreateChannelBuffer(builder_msg, req);
        utils::BuildRequest("/message/channel/new", builder_req, builder_msg);
        
        utils::ClientSendAndRead(context::SocketFd,
                                 builder_req,
                                 [&next_screen](char* buffer, ssize_t n_bytes)->void{
                                     //Parsing response
                                     if(n_bytes < 0){
                                         std::cout << "Communication Error" << std::endl;
                                         return;
                                     }
                                     
                                     auto* resp = fbs::msg::GetCreateChannelResponse(buffer);
                                     if(resp->status_code() != fbs::Status_OK){
                                         std::cout << "Error: ";
                                     }else{
                                         context::msg::CurrentChannelId = resp->channel_id()->str();
                                         next_screen = context::Screen::MSG_MAIN_WINDOW;
                                     }
                                     
                                     std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                 });
        
        return next_screen;
    };
    
    const context::ScreenHandler handleViewChannels = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::MSG_ENTRY;
        
        flatbuffers::FlatBufferBuilder builder_req, builder_channel;
        auto session = fbs::CreateSession(builder_channel,
                                          builder_channel.CreateString(context::CurrentTokenStr));
        auto req = fbs::msg::CreateViewChannelRequest(builder_channel,session);
        fbs::msg::FinishViewChannelRequestBuffer(builder_channel, req);
        utils::BuildRequest("/message/channel", builder_req, builder_channel);
        
        utils::ClientSendAndRead(context::SocketFd,
                                 builder_req,
                                 [&next_screen](char* buffer, ssize_t n_bytes)->void{
                                     //Parsing response
                                     if(n_bytes < 0){
                                         std::cout << "Communication Error" << std::endl;
                                         return;
                                     }
                                     
                                     auto* resp = fbs::msg::GetViewChannelResponse(buffer);
                                     if(resp->status_code() != fbs::Status_OK){
                                         std::cout << "Error: ";
                                         std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                         return;
                                     }
                                     
                                     std::cout << "Result count: "
                                     << (int)resp->channels()->Length() << std::endl;
                                     using context::msg::Channels;
                                     using context::msg::GroupChannelIndex;
                                     Channels.clear();
                                     GroupChannelIndex.clear();
                                     int index = 1;
                                     for(auto* channel : *(resp->channels())){
                                         Channels.push_back(channel->channel_id()->str());
                                         std::cout << "\t<" << index++ << "> ";
                                         std::cout << channel->display_name()->str();
                                         if(channel->type() == fbs::msg::ChannelType_GROUP){
                                             std::cout << " [group]";
                                             GroupChannelIndex.insert(index - 1);
                                         }
                                         std::cout << std::endl;
                                     }
                                     
                                     if(index == 1){
                                         //No channel
                                     }else{
                                         std::cout << "[J]oin\t" << "[L]eave Group" << std::endl;
                                     }
                                     std::cout << "[B]ack" << std::endl;
                                     std::cout << context::PROMPT_CHAR;
                                     
                                     char input_char;
                                     std::cin >> input_char;
                                     switch(std::tolower(input_char)){
                                         case 'b':{
                                             next_screen = context::Screen::MSG_ENTRY;
                                             break;
                                         }
                                             
                                         case 'j':{
                                             next_screen = context::Screen::MSG_JOIN_CHANNEL;
                                             break;
                                         }
                                             
                                         case 'l':{
                                             next_screen = context::Screen::MSG_LEAVE_GROUP;
                                             break;
                                         }
                                             
                                         default:
                                             std::cout << "Unrecognized command: " << input_char << std::endl;
                                     }
                                 });
        
        return next_screen;
    };
    
    const context::ScreenHandler handleJoinChannel = SCREEN_HANDLER(){
        
        auto next_screen = context::Screen::STAY;
        
        using namespace context::msg;
        auto length = Channels.size();
        std::cout << "[1-" << (int)length << "]: ";
        int index;
        std::cin >> index;
        
        if(index < 1 || index > length){
            std::cout << "Error: Index Out Of Range" << std::endl;
        }else{
            CurrentChannelId = Channels[index-1];
            next_screen = context::Screen::MSG_MAIN_WINDOW;
        }
        
        return next_screen;
    };
    
    const context::ScreenHandler handleJoinGroup = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::MSG_ENTRY;
        
        std::cout << "Enter Group Name: ";
        std::string group_name("");
        std::cin >> group_name;
        
        flatbuffers::FlatBufferBuilder builder_group, builder_req;
        auto session = fbs::CreateSession(builder_group,
                                          builder_group.CreateString(context::CurrentTokenStr));
        auto req = fbs::msg::CreateJoinGroupRequest(builder_group,
                                                    session,
                                                    builder_group.CreateString(group_name));
        fbs::msg::FinishJoinGroupRequestBuffer(builder_group, req);
        utils::BuildRequest("/message/group/join", builder_req, builder_group);
        
        utils::ClientSendAndRead(context::SocketFd, builder_req,
                                 [&next_screen](char* buffer, ssize_t n_bytes)->void{
                                     //Parsing response
                                     if(n_bytes < 0){
                                         std::cout << "Communication Error" << std::endl;
                                         return;
                                     }
                                     
                                     auto* resp = fbs::GetGeneralResponse(buffer);
                                     if(resp->status_code() != fbs::Status_OK){
                                         std::cout << "Error: ";
                                     }else{
                                         next_screen = context::Screen::MSG_VIEW_CHANNELS;
                                     }
                                     
                                     std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                 });
        
        return next_screen;
    };
    
    const context::ScreenHandler handleLeaveGroup = SCREEN_HANDLER(){
        
        auto next_screen = context::Screen::STAY;
        
        using namespace context::msg;
        std::cout << "Enter index: ";
        int index = 0;
        std::cin >> index;
        if(GroupChannelIndex.find((size_t)index) != GroupChannelIndex.end()){
            auto channel_id = Channels[index - 1];
            
            flatbuffers::FlatBufferBuilder builder_leave, builder_req;
            auto session = fbs::CreateSession(builder_leave,
                                              builder_leave.CreateString(context::CurrentTokenStr));
            auto req = fbs::msg::CreateLeaveGroupRequest(builder_leave,
                                                         session,
                                                         builder_leave.CreateString(channel_id));
            fbs::msg::FinishLeaveGroupRequestBuffer(builder_leave, req);
            
            utils::BuildRequest("/message/group/leave", builder_req, builder_leave);
            
            utils::ClientSendAndRead(context::SocketFd, builder_req,
                                     [&next_screen](char* buffer, ssize_t n_bytes)->void{
                                         //Parsing response
                                         if(n_bytes < 0){
                                             std::cout << "Communication Error" << std::endl;
                                             next_screen = context::Screen::MSG_ENTRY;
                                             return;
                                         }
                                         
                                         auto* resp = fbs::GetGeneralResponse(buffer);
                                         if(resp->status_code() != fbs::Status_OK){
                                             std::cout << "Error: ";
                                             next_screen = context::Screen::MSG_ENTRY;
                                         }else{
                                             next_screen = context::Screen::MSG_VIEW_CHANNELS;
                                         }
                                         
                                         std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                     });
            
        }else{
            std::cout << "Error: Not A Group" << std::endl;
        }
        
        return next_screen;
    };
    
    const context::ScreenHandler handleMsgMainWindow = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::MSG_ENTRY;
        
        std::cin.sync_with_stdio();
        typedef struct timeval TimeVal;
        
        fd_set fds;
        int stdin_fd = fileno(stdin);
        FD_ZERO(&fds);
        
        const int LONG_POLL_CYCLE = 10000; // 10ms
        const int SHORT_POLL_CYCLE = 100; // 0.1ms
        const int REALLY_SHORT_POLL_CYCLE = 10; // 10us
        int current_timeout_us = SHORT_POLL_CYCLE;
        
        bool terminate = false;
        
        /*Message response handler*/
        const auto msg_resp_callback = [&](char *buffer, ssize_t n_bytes)->void{
            /*Buffer had verified*/
            auto* resp = fbs::msg::GetGetMsgResponse(buffer);
            
            if(resp->status_code() != fbs::Status_OK) return;
            
            current_timeout_us = LONG_POLL_CYCLE;
            for(const auto* msg : *(resp->msg_list())){
                const auto* content_ptr = msg->content();
                switch(msg->content_type()){
                    case fbs::msg::MsgContent_StringContent:{
                        const auto* str_content = reinterpret_cast<const fbs::msg::StringContent*>(content_ptr);
                        std::cout << std::endl << msg->sender()->str() << " -> " ;
                        std::cout << str_content->data()->str() << std::endl;
                        current_timeout_us = SHORT_POLL_CYCLE;
                        break;
                    }
                        
                    case fbs::msg::MsgContent_FileChunk:{
                        
                        const auto* file_chunk = reinterpret_cast<const fbs::msg::FileChunk*>(content_ptr);
                        const auto& file_name = file_chunk->file_name()->str();
                        
                        using context::msg::TransferFdMap;
                        if(TransferFdMap.find(file_name) == TransferFdMap.end()){
                            //Open new file to write
                            auto* file_ptr = ::fopen(file_name.c_str(), "wb");
                            TransferFdMap[file_name] = fileno(file_ptr);
                        }
                        
                        //Check whether EOF
                        if(file_chunk->seq() < 0){
                            close(TransferFdMap[file_name]);
                            TransferFdMap.erase(file_name);
                            std::cout << "Done" << std::endl;
                            current_timeout_us = LONG_POLL_CYCLE;
                        }else{
                            
                            if(file_chunk->seq() == 1){
                                std::cout << std::endl << msg->sender()->str() << " -> " ;
                                std::cout << "Sending " << file_name << " ..." << std::endl;
                            }
                            
                            const auto* data = file_chunk->data();
                            int write_fd = TransferFdMap[file_name];
                            write(write_fd, data->Data(), data->size());
                            std::cout << "=";
                            std::cout.flush();
                            
                            current_timeout_us = REALLY_SHORT_POLL_CYCLE;
                        }
                        break;
                    }
                        
                    case fbs::msg::MsgContent_NONE: {
                        break;
                    }
                }
            }
        };
        
        /*Function for sending file*/
        const auto send_file_handler = [&](const std::string& file_name)->void{
            auto* file_ptr = ::fopen(file_name.c_str(), "rb");
            if(file_ptr == nullptr){
                std::cout << "Error: Failed opening " << file_name << std::endl;
                return;
            }
            
            int fd = fileno(file_ptr);
            char read_file_buffer[FILE_CHUNK_SIZE];
            //memset(read_file_buffer, 0, sizeof(read_file_buffer));
            ssize_t n_bytes;
            int64_t seq_counter = 1;
            while( (n_bytes = read(fd, read_file_buffer, FILE_CHUNK_SIZE)) > 0){
                
                flatbuffers::FlatBufferBuilder builder_req, builder_file;
                
                auto session = fbs::CreateSession(builder_file,
                                                  builder_file.CreateString(context::CurrentTokenStr));
                auto file_data_offset = builder_file.CreateVector((int8_t*)read_file_buffer, n_bytes);
                auto file_content = fbs::msg::CreateFileChunk(builder_file,
                                                              builder_file.CreateString(file_name),
                                                              seq_counter++,
                                                              file_data_offset);
                auto msg_entity = fbs::msg::CreateMsgEntity(builder_file,
                                                            builder_file.CreateString(context::Username),
                                                            fbs::msg::MsgContent_FileChunk,
                                                            file_content.Union());
                const auto& channel_id = context::msg::CurrentChannelId;
                auto req = fbs::msg::CreatePutMsgRequest(builder_file,
                                                         session,
                                                         builder_file.CreateString(channel_id),
                                                         msg_entity);
                fbs::msg::FinishPutMsgRequestBuffer(builder_file, req);
                
                utils::BuildRequest("/message/put", builder_req, builder_file);
                
                utils::ClientSendAndRead(context::SocketFd,
                                         builder_req,
                                         [](char*,ssize_t n_bytes)->void{
                                             if(n_bytes < 0){
                                                 std::cout << "Error: Send File Chunk Failed" << std::endl;
                                             }
                                         });
                
                //memset(read_file_buffer, 0, sizeof(read_file_buffer));
            }
            //EOF or error
            //Send negative sequence number
            flatbuffers::FlatBufferBuilder builder_req, builder_file;
            
            auto session = fbs::CreateSession(builder_file,
                                              builder_file.CreateString(context::CurrentTokenStr));
            auto file_content = fbs::msg::CreateFileChunk(builder_file,
                                                          builder_file.CreateString(file_name),
                                                          -1);
            auto msg_entity = fbs::msg::CreateMsgEntity(builder_file,
                                                        builder_file.CreateString(context::Username),
                                                        fbs::msg::MsgContent_FileChunk,
                                                        file_content.Union());
            const auto& channel_id = context::msg::CurrentChannelId;
            auto req = fbs::msg::CreatePutMsgRequest(builder_file,
                                                     session,
                                                     builder_file.CreateString(channel_id),
                                                     msg_entity);
            fbs::msg::FinishPutMsgRequestBuffer(builder_file, req);
            
            utils::BuildRequest("/message/put", builder_req, builder_file);
            
            utils::ClientSendAndRead(context::SocketFd,
                                     builder_req,
                                     [](char*,ssize_t n_bytes)->void{
                                         if(n_bytes < 0){
                                             std::cout << "Error: Send File Chunk Failed" << std::endl;
                                         }
                                     });
            
            
            ::fclose(file_ptr);
        };
        
        while(!terminate){
            int nready;
            FD_SET(context::SocketFd, &fds);
            FD_SET(stdin_fd, &fds);
            
            TimeVal timeout{0};
            timeout.tv_usec = current_timeout_us;
            
            int maxfd = context::SocketFd + 1;
            if((nready = ::select(maxfd, &fds, nullptr, nullptr, &timeout)) < 0){
                if(errno == EINTR) continue;
                else Log::W("Main Message Window") << "Select error" << std::endl;
            }else if(nready == 0){
                //Timeout
                //Poll
                
                flatbuffers::FlatBufferBuilder builder_msg, builder_req;
                auto session = fbs::CreateSession(builder_msg,
                                                  builder_msg.CreateString(context::CurrentTokenStr));
                auto current_channel = context::msg::CurrentChannelId;
                auto req = fbs::msg::CreateGetMsgRequest(builder_msg,
                                                         session,
                                                         builder_msg.CreateString(current_channel));
                fbs::msg::FinishGetMsgRequestBuffer(builder_msg, req);
                utils::BuildRequest("/message/get", builder_req, builder_msg);
                
                write(context::SocketFd, builder_req.GetBufferPointer(), builder_req.GetSize());
                
            }else{
                if(FD_ISSET(stdin_fd, &fds)){
                    //Keyboard Input
                    
                    std::string input_line("");
                    std::getline(std::cin, input_line);
                    utils::TrimString(input_line, '\n');
                    utils::TrimString(input_line, '\r');
                    if(input_line.length() <= 0){
                        //Previous new line left
                        //Read Again
                        std::getline(std::cin, input_line);
                        utils::TrimString(input_line, '\n');
                        utils::TrimString(input_line, '\r');
                    }
                    
                    /*
                     TODO: Special control
                     */
                    const char* QUIT_KEYWORD = "@quit";
                    const char* SEND_FILE_KEYWORD = "@sendfile ";
                    
                    if(input_line == QUIT_KEYWORD){
                        //Cleanup
                        FD_CLR(context::SocketFd, &fds);
                        FD_CLR(stdin_fd, &fds);
                        terminate = true;
                        continue;
                    }else if(input_line.find(SEND_FILE_KEYWORD) == 0){
                        const auto file_name = input_line.substr(::strlen(SEND_FILE_KEYWORD));
                        send_file_handler(file_name);
                        continue;
                    }
                    
                    flatbuffers::FlatBufferBuilder builder_msg, builder_req;
                    auto session = fbs::CreateSession(builder_msg,
                                                      builder_msg.CreateString(context::CurrentTokenStr));
                    
                    auto str_content = fbs::msg::CreateStringContent(builder_msg,
                                                                     builder_msg.CreateString(input_line));
                    auto msg_content = fbs::msg::CreateMsgEntity(builder_msg,
                                                                 builder_msg.CreateString(context::Username),
                                                                 fbs::msg::MsgContent_StringContent,
                                                                 str_content.Union());
                    
                    auto current_channel = context::msg::CurrentChannelId;
                    auto req = fbs::msg::CreatePutMsgRequest(builder_msg,
                                                             session,
                                                             builder_msg.CreateString(current_channel),
                                                             msg_content);
                    fbs::msg::FinishPutMsgRequestBuffer(builder_msg, req);
                    utils::BuildRequest("/message/put", builder_req, builder_msg);
                    
                    //write(context::SocketFd, builder_req.GetBufferPointer(), builder_req.GetSize());
                    /*
                     Use the re-send feature of the below routine
                     Blocking
                     */
                    utils::ClientSendAndRead(context::SocketFd,
                                             builder_req,
                                             [](char*,ssize_t n_bytes)->void{
                                                 if(n_bytes < 0){
                                                     std::cout << "Error: Send Message Failed" << std::endl;
                                                 }
                                             });
                }
                
                if(FD_ISSET(context::SocketFd, &fds)){
                    //Socket input
                    
                    char recv_buffer[RECV_BUFFER_SIZE];
                    ssize_t n_read = read(context::SocketFd, recv_buffer, RECV_BUFFER_SIZE);
                    if(n_read < 0){
                        Log::E("Main Message Window") << "Read socket error" << std::endl;
                        continue;
                    }
                    
                    flatbuffers::Verifier verifier((uint8_t*)recv_buffer, (size_t)n_read);
                    if(fbs::msg::VerifyGetMsgResponseBuffer(verifier)){
                        msg_resp_callback(recv_buffer, n_read);
                    }else{
                        /*Don't care*/
                    }
                }
            }
        }
        
        return next_screen;
    };
    
    void InitScreens(){
        context::AddScreen(context::Screen::MSG_ENTRY, handleMsgEntry);
        context::AddScreen(context::Screen::MSG_MAIN_WINDOW, handleMsgMainWindow);
        context::AddScreen(context::Screen::MSG_CREATE_PRIVATE_CHANNEL, handleCreateChannel);
        context::AddScreen(context::Screen::MSG_CREATE_GROUP_CHANNEL, handleCreateChannel);
        context::AddScreen(context::Screen::MSG_VIEW_CHANNELS, handleViewChannels);
        context::AddScreen(context::Screen::MSG_JOIN_CHANNEL, handleJoinChannel);
        context::AddScreen(context::Screen::MSG_LEAVE_GROUP, handleLeaveGroup);
        context::AddScreen(context::Screen::MSG_JOIN_GROUP, handleJoinGroup);
    }
}