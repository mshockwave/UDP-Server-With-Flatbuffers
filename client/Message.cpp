#include "Message.hpp"

#include <string>
#include <vector>

#include <boost/tokenizer.hpp>

namespace msg {
    
    const context::ScreenHandler handleMsgEntry = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::MAIN;
        
        std::cout << "[G]roup Chat\t" << "[P]rivate Chat" << std::endl;
        std::cout << "[C]onversations" << std::endl;
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
            
            std::cout << "Enter members(separated by comma):" << std::endl;
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
                                     Channels.clear();
                                     int index = 1;
                                     for(auto* channel : *(resp->channels())){
                                         Channels.push_back(channel->channel_id()->str());
                                         std::cout << "\t<" << index++ << "> ";
                                         std::cout << channel->display_name()->str();
                                         if(channel->type() == fbs::msg::ChannelType_GROUP){
                                             std::cout << " [group]";
                                         }
                                         std::cout << std::endl;
                                     }
                                     
                                     if(index == 1){
                                         //No channel
                                     }else{
                                         std::cout << "[J]oin\t" << "[L]eave" << std::endl;
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
                                             
                                         default:
                                             std::cout << "Unrecognized command: " << input_char << std::endl;
                                     }
                                 });
        
        return next_screen;
    };
    
    const context::ScreenHandler handleMsgMainWindow = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        //TODO
        auto next_screen = context::Screen::MSG_ENTRY;
        std::cout << "This is message main window" << std::endl;
        
        return next_screen;
    };
    
    void InitScreens(){
        context::AddScreen(context::Screen::MSG_ENTRY, handleMsgEntry);
        context::AddScreen(context::Screen::MSG_MAIN_WINDOW, handleMsgMainWindow);
        context::AddScreen(context::Screen::MSG_CREATE_PRIVATE_CHANNEL, handleCreateChannel);
        context::AddScreen(context::Screen::MSG_CREATE_GROUP_CHANNEL, handleCreateChannel);
        context::AddScreen(context::Screen::MSG_VIEW_CHANNELS, handleViewChannels);
    }
}