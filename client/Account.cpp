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
        const auto response_callback = [&next_screen](char *response, ssize_t n_bytes)->void{
            
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
        const auto response_callback = [&next_screen](char *response, ssize_t n_bytes)->void{
            
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
    
    void InitScreens(){
        context::AddScreen(context::Screen::REGISTER, handleRegister);
        context::AddScreen(context::Screen::LOGIN, handleLogin);
        context::AddScreen(context::Screen::LOGOUT, handleLogout);
    }
    
} //namespace account