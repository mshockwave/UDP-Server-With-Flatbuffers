#include <vector>

#include "Post.hpp"
#include "Context.hpp"

#include <Log.hpp>
#include <Utils.hpp>
#include <schemas/post.h>

namespace post {
    
    const context::ScreenHandler handleAddPost = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        std::string content;
        std::cout << "Enter Content: ";
        std::cin.ignore(); //Ignore return characters
        std::getline(std::cin, content);
        
        flatbuffers::FlatBufferBuilder builder_new_post, builder_req;
        
        std::vector< flatbuffers::Offset<flatbuffers::String> > dummy_granted;
        auto post_perm = fbs::post::CreatePostPermission(builder_new_post,
                                                         fbs::post::PostPermissionType_ANY,
                                                         builder_new_post.CreateVector(dummy_granted));
        auto session_fbs = fbs::CreateSession(builder_new_post,
                                              builder_new_post.CreateString(context::CurrentTokenStr));
        
        auto new_post_req = fbs::post::CreateNewPostRequest(builder_new_post,
                                                            session_fbs,
                                                            builder_new_post.CreateString(content),
                                                            post_perm);
        fbs::post::FinishNewPostRequestBuffer(builder_new_post, new_post_req);
        
        utils::BuildRequest("/post/new", builder_req, builder_new_post);
        
        auto next_screen = context::Screen::MAIN;
        utils::ClientSendAndRead(context::SocketFd,
                                 builder_req, [&next_screen](char* buffer, ssize_t n_bytes)->void{
                                     //Parsing response
                                     if(n_bytes < 0){
                                         std::cout << "Communication Error" << std::endl;
                                         next_screen = context::Screen::STAY;
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
    
    const context::ScreenHandler handleViewPost = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::STAY;
        
        if(context::post::CurrentPid <= 0){
            //Retrieve max pid
            flatbuffers::FlatBufferBuilder builder_get_max_pid, builder_req;
            
            auto session_fbs = fbs::CreateSession(builder_get_max_pid,
                                                  builder_get_max_pid.CreateString(context::CurrentTokenStr));
            
            auto get_max_pid_req = fbs::post::CreateGetMaxPidRequest(builder_get_max_pid,
                                                                     session_fbs);
            fbs::post::FinishGetMaxPidRequestBuffer(builder_get_max_pid, get_max_pid_req);
            
            utils::BuildRequest("/post/view/maxPid", builder_req, builder_get_max_pid);
            
            auto max_pid_req_callback = [&](char* buffer, ssize_t n_bytes)->void{
                //Parsing response
                if(n_bytes < 0){
                    std::cout << "Communication Error" << std::endl;
                    next_screen = context::Screen::MAIN;
                    return;
                }
                
                auto* resp = fbs::post::GetGetMaxPidResponse(buffer);
                auto status_code = resp->status_code();
                if(status_code != fbs::Status_OK){
                    std::cout << "Error: " << utils::GetErrorVerbose(status_code) << std::endl;
                    next_screen = context::Screen::MAIN;
                }else{
                    context::post::CurrentPid = resp->max_post_id();
                    context::post::MaxPid = context::post::CurrentPid;
                }
            };
            
            utils::ClientSendAndRead(context::SocketFd,
                                     builder_req,
                                     max_pid_req_callback);
        }
        
        if(context::post::CurrentPid <= 0){
            //Fail
            return next_screen;
        }
        
        //Get Post
        flatbuffers::FlatBufferBuilder builder_view_post, builder_req;
        
        auto session_fbs = fbs::CreateSession(builder_view_post,
                                              builder_view_post.CreateString(context::CurrentTokenStr));
        
        auto edit_post_req = fbs::post::CreateGetPostRequest(builder_view_post,
                                                             session_fbs,
                                                             (uint64_t)context::post::CurrentPid);
        fbs::post::FinishGetPostRequestBuffer(builder_view_post, edit_post_req);
        
        utils::BuildRequest("/post/view", builder_req, builder_view_post);
        
        utils::ClientSendAndRead(context::SocketFd, builder_req, [&](char* buffer, ssize_t n_bytes)->void{
            //Parsing response
            if(n_bytes < 0){
                std::cout << "Communication Error" << std::endl;
                next_screen = context::Screen::MAIN;
                return;
            }
            
            auto* resp = fbs::post::GetGetPostResponse(buffer);
            auto status_code = resp->status_code();
            if(status_code != fbs::Status_OK){
                std::cout << "Error: " << utils::GetErrorVerbose(status_code) << std::endl;
                next_screen = context::Screen::MAIN;
            }else{
                std::cout << "Poster: " << resp->poster_name()->str() << " "
                            << resp->poster_nickname()->str() << " "
                            << resp->poster_addr()->str() << std::endl;
                
                std::cout << "Content: " << resp->content()->str() << std::endl;
                
                std::cout << "Post Time: " << resp->timestamp()->str() << std::endl;
                
                std::cout << "Like number: " << (int)resp->like_num() << std::endl;
            }
        });
        
        std::cout << std::endl;
        if(context::post::CurrentPid > 1){
            std::cout << "[P]revious Post\t";
        }
        if(context::post::CurrentPid < context::post::MaxPid){
            std::cout << "[N]ext Post" << std::endl;
        }
        std::cout << "[B]ack" << std::endl;
        
        std::cout << context::PROMPT_CHAR;
        char input_char;
        std::cin >> input_char;
        
        switch(std::tolower(input_char)){
            case 'n':{
                if(context::post::CurrentPid < context::post::MaxPid){
                    context::post::CurrentPid++;
                }else{
                    std::cout << "Unrecognized command: " << input_char << std::endl;
                }
                break;
            }
                
            case 'p':{
                if(context::post::CurrentPid > 1){
                    context::post::CurrentPid--;
                }else{
                    std::cout << "Unrecognized command: " << input_char << std::endl;
                }
                break;
            }
                
            case 'b':{
                context::post::CurrentPid = -1;
                next_screen = context::Screen::MAIN;
                break;
            }
        }
        
        return next_screen;
    };
    
    void InitScreens(){
        context::AddScreen(context::Screen::ADD_POST, handleAddPost);
        context::AddScreen(context::Screen::VIEW_POST, handleViewPost);
    }
    
} //namespace post