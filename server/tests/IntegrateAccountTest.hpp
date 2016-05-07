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
        
        flatbuffers::FlatBufferBuilder builder_register;
        
        auto register_req = fbs::account::CreateRegisterRequest(builder_register,
                                                                builder_register.CreateString("mac"),
                                                                builder_register.CreateString("abcde"));
        fbs::account::FinishRegisterRequestBuffer(builder_register, register_req);
        
        flatbuffers::FlatBufferBuilder builder_raw;
        auto payload = builder_raw.CreateVector((int8_t*)builder_register.GetBufferPointer(),
                                                builder_register.GetSize());
        auto raw_req = fbs::CreateRequestPacket(builder_raw,
                                                builder_raw.CreateString("/account/register"),
                                                payload);
        fbs::FinishRequestPacketBuffer(builder_raw, raw_req);
        
        if( write(sockFd, builder_raw.GetBufferPointer(), builder_raw.GetSize()) < 0){
            Log::E(mName) << "Fail sending register request to server" << std::endl;
            return false;
        }
        
        char recv_buffer[RECV_BUFFER_SIZE];
        memset(recv_buffer, 0, sizeof(recv_buffer));
        
        ssize_t n_bytes;
        if( (n_bytes = read(sockFd, recv_buffer, RECV_BUFFER_SIZE)) < 0){
            Log::E(mName) << "Fail reading register response from server" << std::endl;
            //perror("Reading from server: ");
            return false;
        }
        
        //Parsing response
        auto resp = fbs::GetGeneralResponse(recv_buffer);
        Log::V(mName) << "Response status: " << (int)resp->status_code() << std::endl;
        if(resp->status_code() == 0){
            auto* session = resp->session();
            Log::V(mName) << "Response token: " << session->token()->str() << std::endl;
        }
        
        //Log::V(mName) << "Finish register testing" << std::endl;
        
        return true;
    }
    
    bool doClient(){
        
        Log::V(mName) << "Connecting to localhost:" << TEST_SERVER_PORT << "..." << std::endl;
        int sockFd = utils::udp_connect("127.0.0.1", TEST_SERVER_PORT);
        Assert(sockFd >= 0, "Udp client failed connecting");
        
        bool result = true;
        
        result &= testRegister(sockFd);
        
        close(sockFd);
        
        return result;
    }
};

#endif