#ifndef _INTEGRATE_ACCOUNT_TEST_HPP_
#define _INTEGRATE_ACCOUNT_TEST_HPP_

#include <cstring>

#include <IntegrateSerCliTest.hpp>
#include <Utils.hpp>
#include "../Types.hpp"

#include <schemas/account.h>

class IntegrateAccountTest : public DefaultIntegrateSerCliTest {
public:
    IntegrateAccountTest() :
        DefaultIntegrateSerCliTest("IntegrateAccountTest"){}
    
private:
    
    bool testRegister(int sockFd){
        
        Log::V(mName) << "Testing Register..." << std::endl;
        
        flatbuffers::FlatBufferBuilder builder_register;
        
        auto register_req = fbs::account::CreateRegisterRequest(builder_register,
                                                                builder_register.CreateString("mac"),
                                                                builder_register.CreateString("abcde"));
        fbs::account::FinishRegisterRequestBuffer(builder_register, register_req);
        
        flatbuffers::FlatBufferBuilder builder_req;
        utils::BuildRequest("/account/register", builder_req, builder_register);
        
        if( write(sockFd, builder_req.GetBufferPointer(), builder_req.GetSize()) < 0){
            Log::E(mName) << "Fail sending register request to server" << std::endl;
            perror("Writing to server: ");
            return false;
        }
        
        char recv_buffer[RECV_BUFFER_SIZE];
        memset(recv_buffer, 0, sizeof(recv_buffer));
        
        ssize_t n_bytes;
        if( (n_bytes = read(sockFd, recv_buffer, RECV_BUFFER_SIZE)) < 0){
            Log::E(mName) << "Fail reading register response from server" << std::endl;
            perror("Reading from server: ");
            return false;
        }
        
        //Parsing response
        auto resp = fbs::GetGeneralResponse(recv_buffer);
        Log::V(mName) << "Response status: " << (int)resp->status_code() << std::endl;
        if(resp->status_code() == 0){
            auto* session = resp->session();
            Log::V(mName) << "Response token: " << session->token()->str() << std::endl;
        }
        
        return true;
    }
    
    bool testLogin(int sockFd){
        
        Log::V(mName) << "Testing Login..." << std::endl;
        
        flatbuffers::FlatBufferBuilder builder_login;
        
        auto login_req = fbs::account::CreateLoginRequest(builder_login,
                                                          builder_login.CreateString("mac"),
                                                          builder_login.CreateString("abcde"));
        fbs::account::FinishLoginRequestBuffer(builder_login, login_req);
        
        flatbuffers::FlatBufferBuilder builder_req;
        utils::BuildRequest("/account/login", builder_req, builder_login);
        
        if( write(sockFd, builder_req.GetBufferPointer(), builder_req.GetSize()) < 0){
            Log::E(mName) << "Fail sending login request to server" << std::endl;
            perror("Writing to server: ");
            return false;
        }
        
        char recv_buffer[RECV_BUFFER_SIZE];
        memset(recv_buffer, 0, sizeof(recv_buffer));
        
        ssize_t n_bytes;
        if( (n_bytes = read(sockFd, recv_buffer, RECV_BUFFER_SIZE)) < 0){
            Log::E(mName) << "Fail reading login response from server" << std::endl;
            perror("Reading from server: ");
            return false;
        }
        
        //Parsing response
        auto resp = fbs::GetGeneralResponse(recv_buffer);
        Log::V(mName) << "Response status: " << (int)resp->status_code() << std::endl;
        if(resp->status_code() == 0){
            auto* session = resp->session();
            Log::V(mName) << "Response token: " << session->token()->str() << std::endl;
        }
        
        return true;
    }
    
    bool doClient(){
        
        Log::V(mName) << "Connecting to localhost:" << TEST_SERVER_PORT << "..." << std::endl;
        int sockFd = utils::udp_connect("127.0.0.1", TEST_SERVER_PORT);
        Assert(sockFd >= 0, "Udp client failed connecting");
        
        bool result = true;
        
        result &= testRegister(sockFd);
        result &= testLogin(sockFd);
        
        close(sockFd);
        
        return result;
    }
};

#endif