#include "Account.hpp"

#include <schemas/account.h>

namespace account {
    
    const context::ScreenHandler handleRegister = SCREEN_HANDLER(){
        std::string account, password;
        std::cout << "Enter account: ";
        std::cin >> account;
        std::cout << "Enter password: ";
        std::cin >> password;
        
        flatbuffers::FlatBufferBuilder builder_reg, builder_raw;
        auto reg_req = fbs::account::CreateRegisterRequest(builder_reg,
                                                           builder_reg.CreateString(account),
                                                           builder_reg.CreateString(password));
        fbs::account::FinishRegisterRequestBuffer(builder_reg, reg_req);
        
        utils::BuildRequest("/account/register", builder_raw, builder_reg);
        
        auto next_screen = context::Screen::MAIN;
        const auto response_callback = [&next_screen, account](char *response, ssize_t n_bytes)->void{
            
            if(n_bytes < 0){
                std::cout << "Communication Error" << std::endl;
                next_screen = context::Screen::STAY;
                return;
            }
            
            flatbuffers::Verifier verifier((uint8_t*)response, static_cast<size_t>(n_bytes));
            if(fbs::VerifyGeneralResponseBuffer(verifier)){
                auto* resp = fbs::GetGeneralResponse(response);
                if(resp->status_code() != fbs::Status_OK){
                    std::cout << "Error: ";
                    next_screen = context::Screen::ENTRY;
                }else{
                    const auto& token_str = resp->session()->token()->str();
                    context::CurrentTokenStr = token_str;
                    context::Username = account;
                }
                std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
            }else{
                std::cout << "Error: "
                << utils::GetErrorVerbose(fbs::Status_PAYLOAD_FORMAT_INVALID)
                << std::endl;
                next_screen = context::Screen::ENTRY;
            }
            
        };
        utils::ClientSendAndRead(context::SocketFd,
                                 builder_raw,
                                 response_callback);
        
        return next_screen;
    };
    
    const context::ScreenHandler handleLogin = SCREEN_HANDLER(){
        std::string account, password;
        std::cout << "Enter account name: ";
        std::cin >> account;
        std::cout << "Enter password: ";
        std::cin >> password;
        
        flatbuffers::FlatBufferBuilder builder_login, builder_raw;
        auto login_req = fbs::account::CreateLoginRequest(builder_login,
                                                          builder_login.CreateString(account),
                                                          builder_login.CreateString(password));
        fbs::account::FinishLoginRequestBuffer(builder_login, login_req);
        
        utils::BuildRequest("/account/login", builder_raw, builder_login);
        
        auto next_screen = context::Screen::MAIN;
        const auto response_callback = [&next_screen, account](char *response, ssize_t n_bytes)->void{
            
            if(n_bytes < 0){
                std::cout << "Communication Error" << std::endl;
                next_screen = context::Screen::STAY;
                return;
            }
            
            flatbuffers::Verifier verifier((uint8_t*)response, static_cast<size_t>(n_bytes));
            if(fbs::VerifyGeneralResponseBuffer(verifier)){
                auto* resp = fbs::GetGeneralResponse(response);
                if(resp->status_code() != fbs::Status_OK){
                    std::cout << "Error: ";
                    next_screen = context::Screen::ENTRY;
                }else{
                    const auto& token_str = resp->session()->token()->str();
                    context::CurrentTokenStr = token_str;
                    context::Username = account;
                }
                std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
            }else{
                std::cout << "Error: "
                << utils::GetErrorVerbose(fbs::Status_PAYLOAD_FORMAT_INVALID)
                << std::endl;
                next_screen = context::Screen::ENTRY;
            }
        };
        utils::ClientSendAndRead(context::SocketFd,
                                 builder_raw,
                                 response_callback);
        
        return next_screen;
    };
    
    const context::ScreenHandler handleLogout = SCREEN_HANDLER(){
        flatbuffers::FlatBufferBuilder builder_logout, builder_raw;
        auto session = fbs::CreateSession(builder_logout,
                                          builder_logout.CreateString(context::CurrentTokenStr));
        auto logout_req = fbs::account::CreateLogoutRequest(builder_logout, session);
        fbs::account::FinishLogoutRequestBuffer(builder_logout, logout_req);
        
        utils::BuildRequest("/account/logout", builder_raw, builder_logout);
        
        auto next_screen = context::Screen::ENTRY;
        const auto response_callback = [&next_screen](char *response, ssize_t n_bytes)->void{
            
            if(n_bytes < 0){
                std::cout << "Communication Error" << std::endl;
                next_screen = context::Screen::STAY;
                return;
            }
            
            flatbuffers::Verifier verifier((uint8_t*)response, static_cast<size_t>(n_bytes));
            if(fbs::account::VerifyLogoutResponseBuffer(verifier)){
                auto* resp = fbs::account::GetLogoutResponse(response);
                if(resp->status_code() != fbs::Status_OK){
                    std::cout << "Error: ";
                    next_screen = context::Screen::STAY;
                }
                std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
            }else{
                std::cout << "Error: "
                << utils::GetErrorVerbose(fbs::Status_PAYLOAD_FORMAT_INVALID)
                << std::endl;
                next_screen = context::Screen::STAY;
            }
        };
        utils::ClientSendAndRead(context::SocketFd,
                                 builder_raw,
                                 response_callback);
        return next_screen;
    };
    
    const context::ScreenHandler handleSearchAccount = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::MAIN;
        
        std::cout << "Enter username or nickname keyword: ";
        std::string query_string("");
        std::cin >> query_string;
        
        flatbuffers::FlatBufferBuilder builder_search, builder_req;
        
        auto session = fbs::CreateSession(builder_search,
                                          builder_search.CreateString(context::CurrentTokenStr));
        auto search_req = fbs::account::CreateSearchRequest(builder_search,
                                                            session,
                                                            builder_search.CreateString(query_string));
        fbs::account::FinishSearchRequestBuffer(builder_search, search_req);
        
        utils::BuildRequest("/account/search", builder_req, builder_search);
        
        const auto response_callback = [&next_screen](char *response, ssize_t n_bytes)->void{
            
            if(n_bytes < 0){
                std::cout << "Communication Error" << std::endl;
                return;
            }
            
            flatbuffers::Verifier verifier((uint8_t*)response, static_cast<size_t>(n_bytes));
            if(fbs::account::VerifySearchResponseBuffer(verifier)){
                auto* resp = fbs::account::GetSearchResponse(response);
                if(resp->status_code() != fbs::Status_OK){
                    std::cout << "Error: ";
                    std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                }else{
                    std::cout << "Result count: " << (int)resp->results()->Length() << std::endl;
                    const auto& results = *(resp->results());
                    for(auto it_result : results){
                        std::cout << "\t" << it_result->str() << std::endl;
                    }
                }
            }else{
                std::cout << "Error: "
                << utils::GetErrorVerbose(fbs::Status_PAYLOAD_FORMAT_INVALID)
                << std::endl;
            }
        };
        
        utils::ClientSendAndRead(context::SocketFd, builder_req, response_callback);
        
        return next_screen;
    };
    
    const context::ScreenHandler handleFriendEntry = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::MAIN;
        
        std::cout << "[A]dd Friend\t" << "[V]iew Friends" << std::endl;
        std::cout << "View Pending Friend [R]equests" << std::endl;
        std::cout << "[B]ack" << std::endl;
        std::cout << context::PROMPT_CHAR ;
        
        char input_char;
        std::cin >> input_char;
        switch(std::tolower(input_char)){
            case 'a':{
                next_screen = context::Screen::ADD_FRIEND;
                break;
            }
                
            case 'v':{
                next_screen = context::Screen::VIEW_FRIENDS;
                break;
            }
                
            case 'r':{
                next_screen = context::Screen::VIEW_PENDING_FRIENDS;
                break;
            }
                
            case 'b':{
                next_screen = context::Screen::MAIN;
                break;
            }
                
            default:
                std::cout << "Unrecognized command: " << input_char << std::endl;
        }
        
        return next_screen;
    };
    
    const context::ScreenHandler handleAddFriend = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::FRIEND_ENTRY;
        
        std::cout << "Enter Account Name: ";
        std::string friend_name("");
        std::cin >> friend_name;
        
        flatbuffers::FlatBufferBuilder builder_friend, builder_req;
        auto session = fbs::CreateSession(builder_friend,
                                          builder_friend.CreateString(context::CurrentTokenStr));
        
        auto req = fbs::account::CreateAddFriendRequest(builder_friend,
                                                        session,
                                                        builder_friend.CreateString(friend_name));
        fbs::account::FinishAddFriendRequestBuffer(builder_friend, req);
        
        utils::BuildRequest("/account/friend/add", builder_req, builder_friend);
        
        utils::ClientSendAndRead(context::SocketFd,
                                 builder_req, [&next_screen](char* buffer, ssize_t n_bytes)->void{
                                     //Parsing response
                                     if(n_bytes < 0){
                                         std::cout << "Communication Error" << std::endl;
                                         return;
                                     }
                                     
                                     auto* resp = fbs::GetGeneralResponse(buffer);
                                     if(resp->status_code() != fbs::Status_OK){
                                         std::cout << "Error: ";
                                     }
                                     
                                     std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                 });
        
        return next_screen;
    };
    
    const context::ScreenHandler handleViewPendingFriendReq = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::FRIEND_ENTRY;
        
        flatbuffers::FlatBufferBuilder builder_req, builder_friend;
        auto session = fbs::CreateSession(builder_friend,
                                          builder_friend.CreateString(context::CurrentTokenStr));
        
        auto req = fbs::account::CreateViewPendingFriendRequest(builder_friend, session);
        fbs::account::FinishViewPendingFriendRequestBuffer(builder_friend, req);
        
        utils::BuildRequest("/account/friend/pending", builder_req, builder_friend);
        
        utils::ClientSendAndRead(context::SocketFd,
                                 builder_req,
                                 [&next_screen](char* buffer, ssize_t n_bytes)->void{
                                     //Parsing response
                                     if(n_bytes < 0){
                                         std::cout << "Communication Error" << std::endl;
                                         return;
                                     }
                                     
                                     auto* resp = fbs::account::GetViewPendingFriendResponse(buffer);
                                     if(resp->status_code() != fbs::Status_OK){
                                         std::cout << "Error: ";
                                         std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                         return;
                                     }
                                     
                                     const auto& results = *(resp->results());
                                     std::cout << "Result count: " << (int)results.Length() << std::endl;
                                     context::account::PendingFriends.clear();
                                     int index = 1;
                                     for(auto it_result : results){
                                         auto name = it_result->str();
                                         context::account::PendingFriends.push_back(name);
                                         std::cout << "\t[" << index++ << "] " << name << std::endl;
                                     }
                                     
                                     if(index == 1){
                                         //No friend requests QQ
                                     }else{
                                         std::cout << "[C]onfirm Request\t";
                                     }
                                     std::cout << "[B]ack" << std::endl;
                                     
                                     std::cout << context::PROMPT_CHAR;
                                     char input_char;
                                     std::cin >> input_char;
                                     
                                     switch(std::tolower(input_char)){
                                         case 'b':{
                                             next_screen = context::Screen::FRIEND_ENTRY;
                                             break;
                                         }
                                             
                                         case 'c':{
                                             if(index == 1){
                                                 std::cout << "Unrecognized command: " << input_char << std::endl;
                                                 break;
                                             }else{
                                                 next_screen = context::Screen::CONFIRM_PENDING_FRIEND;
                                                 break;
                                             }
                                         }
                                             
                                         default:
                                             std::cout << "Unrecognized command: " << input_char << std::endl;
                                     }
                                 });
        
        return next_screen;
    };
    
    const context::ScreenHandler handleConfirmFriendRequets = SCREEN_HANDLER(){
        
        //context::PrintDivideLine();
        
        auto next_screen = context::Screen::FRIEND_ENTRY;
        
        std::cout << "Enter number: [1-" << std::to_string(context::account::PendingFriends.size()) << "]: ";
        int index = 0;
        std::cin >> index;
        
        if(index < 1 || index > context::account::PendingFriends.size()){
            std::cout << "Error: Index out of range" << std::endl;
            next_screen = context::Screen::STAY;
        }else{
            --index;
            auto friend_name = context::account::PendingFriends[index];
            
            flatbuffers::FlatBufferBuilder builder_friend, builder_req;
            auto session = fbs::CreateSession(builder_friend,
                                              builder_friend.CreateString(context::CurrentTokenStr));
            
            auto req = fbs::account::CreateAddFriendRequest(builder_friend,
                                                            session,
                                                            builder_friend.CreateString(friend_name));
            fbs::account::FinishAddFriendRequestBuffer(builder_friend, req);
            
            utils::BuildRequest("/account/friend/add", builder_req, builder_friend);
            
            utils::ClientSendAndRead(context::SocketFd,
                                     builder_req, [&next_screen](char* buffer, ssize_t n_bytes)->void{
                                         //Parsing response
                                         if(n_bytes < 0){
                                             std::cout << "Communication Error" << std::endl;
                                             return;
                                         }
                                         
                                         auto* resp = fbs::GetGeneralResponse(buffer);
                                         if(resp->status_code() != fbs::Status_OK){
                                             std::cout << "Error: ";
                                         }else{
                                             next_screen = context::Screen::VIEW_PENDING_FRIENDS;
                                         }
                                         
                                         std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                     });
        }
        
        return next_screen;
    };
    
    void InitScreens(){
        context::AddScreen(context::Screen::REGISTER, handleRegister);
        context::AddScreen(context::Screen::LOGIN, handleLogin);
        context::AddScreen(context::Screen::LOGOUT, handleLogout);
        context::AddScreen(context::Screen::SEARCH_ACCOUNT, handleSearchAccount);
        context::AddScreen(context::Screen::FRIEND_ENTRY, handleFriendEntry);
        context::AddScreen(context::Screen::ADD_FRIEND, handleAddFriend);
        context::AddScreen(context::Screen::VIEW_PENDING_FRIENDS, handleViewPendingFriendReq);
        context::AddScreen(context::Screen::CONFIRM_PENDING_FRIEND, handleConfirmFriendRequets);
    }
    
} //namespace account