#ifndef _INTEGRATE_POST_TEST_HPP_
#define _INTEGRATE_POST_TEST_HPP_

#include <cstring>
#include <string>
#include <vector>

#include <IntegrateSerCliTest.hpp>
#include <Utils.hpp>
#include "../Types.hpp"

#include <schemas/account.h>
#include <schemas/post.h>

class IntegratePostTest : public DefaultIntegrateSerCliTest {
public:
    IntegratePostTest() :
        DefaultIntegrateSerCliTest("IntegratePostTest"),
        SessionToken(""){}
    
private:
    
    std::string SessionToken;
    
    bool login(int sockFd){
        
        flatbuffers::FlatBufferBuilder builder_login;
        
        auto login_req = fbs::account::CreateLoginRequest(builder_login,
                                                          builder_login.CreateString("mac"),
                                                          builder_login.CreateString("abcde"));
        fbs::account::FinishLoginRequestBuffer(builder_login, login_req);
        
        flatbuffers::FlatBufferBuilder builder_req;
        utils::BuildRequest("/account/login", builder_req, builder_login);
        
        utils::ClientSendAndRead(sockFd, builder_req, [&](char* buffer, ssize_t n_bytes)->void{
            //Parsing response
            if(n_bytes < 0){
                Log::E(mName) << "Login failed" << std::endl;
                return;
            }
            
            auto* resp = fbs::GetGeneralResponse(buffer);
            //Log::V(mName) << "Response status: " << (int)resp->status_code() << std::endl;
            if(resp->status_code() == fbs::Status_OK){
                auto* session = resp->session();
                SessionToken = session->token()->str();
            }else{
                Log::E(mName) << "Login failed: "
                                << utils::GetErrorVerbose(resp->status_code()) << std::endl;
            }
        });
        
        return (SessionToken.length() > 0);
    }
    
    bool testNewPost(int sockFd){
        
        Log::V(mName) << "Testing New Post..." << std::endl;
        
        flatbuffers::FlatBufferBuilder builder_new_post, builder_req;
        
        std::string content("This is post content");
        
        std::vector< flatbuffers::Offset<flatbuffers::String> > dummy_granted;
        auto post_perm = fbs::post::CreatePostPermission(builder_new_post,
                                                         fbs::post::PostPermissionType_ANY,
                                                         builder_new_post.CreateVector(dummy_granted));
        auto session_fbs = fbs::CreateSession(builder_new_post,
                                              builder_new_post.CreateString(SessionToken));
        
        auto new_post_req = fbs::post::CreateNewPostRequest(builder_new_post,
                                                            session_fbs,
                                                            builder_new_post.CreateString(content),
                                                            post_perm);
        fbs::post::FinishNewPostRequestBuffer(builder_new_post, new_post_req);
        
        utils::BuildRequest("/post/new", builder_req, builder_new_post);
        
        bool post_result = true;
        utils::ClientSendAndRead(sockFd, builder_req, [&](char* buffer, ssize_t n_bytes)->void{
            //Parsing response
            if(n_bytes < 0){
                Log::E(mName) << "New Post failed" << std::endl;
                post_result &= false;
                return;
            }
            
            auto* resp = fbs::GetGeneralResponse(buffer);
            if(resp->status_code() != fbs::Status_OK){
                Log::E(mName) << "New Post failed: "
                << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                post_result &= false;
            }
        });
        
        return post_result;
    }
    
    bool testEditPost(int sockFd){
        Log::V(mName) << "Testing Edit Post..." << std::endl;
        
        flatbuffers::FlatBufferBuilder builder_edit_post, builder_req;
        
        std::string content("This is modified post content");
        
        auto session_fbs = fbs::CreateSession(builder_edit_post,
                                              builder_edit_post.CreateString(SessionToken));
        
        auto edit_post_req = fbs::post::CreateEditPostRequest(builder_edit_post,
                                                              session_fbs,
                                                              (uint64_t)1,
                                                              builder_edit_post.CreateString(content));
        fbs::post::FinishEditPostRequestBuffer(builder_edit_post, edit_post_req);
        
        utils::BuildRequest("/post/edit", builder_req, builder_edit_post);
        
        bool edit_result = true;
        utils::ClientSendAndRead(sockFd, builder_req, [&](char* buffer, ssize_t n_bytes)->void{
            //Parsing response
            if(n_bytes < 0){
                Log::E(mName) << "Edit Post failed" << std::endl;
                edit_result &= false;
                return;
            }
            
            auto* resp = fbs::GetGeneralResponse(buffer);
            if(resp->status_code() != fbs::Status_OK){
                Log::E(mName) << "Edit Post failed: "
                << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                edit_result &= false;
            }
        });
        
        return edit_result;
    }
    
    bool doClient(){
        
        Log::V(mName) << "Connecting to localhost:" << TEST_SERVER_PORT << "..." << std::endl;
        int sockFd = utils::udp_connect("127.0.0.1", TEST_SERVER_PORT);
        Assert(sockFd >= 0, "Udp client failed connecting");
        
        bool result = true;
        
        result &= login(sockFd);
        if(!result){
            close(sockFd);
            return result;
        }
        result &= testNewPost(sockFd);
        result &= testEditPost(sockFd);
        
        close(sockFd);
        
        return result;
    }
};

#endif