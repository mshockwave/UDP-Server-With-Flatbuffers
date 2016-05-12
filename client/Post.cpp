#include <vector>
#include <boost/tokenizer.hpp>

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
        
        bool skip = false;
        bool can_edit = false;
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
                /*
                if(status_code == fbs::Status_PERMISSION_DENIED){
                    skip = true;
                }else{
                    std::cout << "Error: " << utils::GetErrorVerbose(status_code) << std::endl;
                    next_screen = context::Screen::MAIN;
                }
                 */
                skip = true;
            }else{
                
                context::PrintDivideLine();
                
                std::cout << "Poster: " << resp->poster_name()->str() << " "
                            << resp->poster_nickname()->str() << " "
                            << resp->poster_addr()->str() << std::endl;
                can_edit = (resp->poster_name()->str() == context::Username);
                
                std::cout << "Content: " << resp->content()->str() << std::endl;
                
                std::cout << "Post Time: " << resp->timestamp()->str() << std::endl;
                
                std::cout << "Like number: " << (int)resp->like_num() << std::endl;
                
                //Comment info
                context::post::MaxCid = resp->max_comment_id();
                context::post::CurrentCid = context::post::MaxCid;
            }
        });
        
        if(skip){
            switch(current_screen){
                case context::Screen::VIEW_NEXT_POST:{
                    if(context::post::CurrentPid < context::post::MaxPid){
                        context::post::CurrentPid++;
                        return context::Screen::STAY;
                    }else{
                        return context::Screen::MAIN;
                    }
                }
                    
                case context::Screen::VIEW_PREV_POST:{
                    if(context::post::CurrentPid > 1){
                        context::post::CurrentPid--;
                        return context::Screen::STAY;
                    }else{
                        return context::Screen::MAIN;
                    }
                }
                    
                default:
                    break;
            }
        }
        
        std::cout << std::endl;
        if(context::post::CurrentPid > 1){
            std::cout << "[P]revious Post\t";
        }
        if(context::post::CurrentPid < context::post::MaxPid){
            std::cout << "[N]ext Post";
        }
        std::cout <<std::endl;
        std::cout << "[L]ike / [U]nlike This post" << std::endl;
        std::cout << "[A]dd Comment";
        if(context::post::MaxCid > 0){
            std::cout << "\t[V]iew Comments";
        }
        std::cout << std::endl;
        if(can_edit){
            std::cout << "[E]dit\t";
            std::cout << "[R]emove" << std::endl;
        }
        std::cout << "[B]ack" << std::endl;
        
        std::cout << context::PROMPT_CHAR;
        char input_char;
        std::cin >> input_char;
        
        switch(std::tolower(input_char)){
            case 'n':{
                if(context::post::CurrentPid < context::post::MaxPid){
                    context::post::CurrentPid++;
                    next_screen = context::Screen::VIEW_NEXT_POST;
                }else{
                    std::cout << "Unrecognized command: " << input_char << std::endl;
                }
                break;
            }
                
            case 'p':{
                if(context::post::CurrentPid > 1){
                    context::post::CurrentPid--;
                    next_screen = context::Screen::VIEW_PREV_POST;
                }else{
                    std::cout << "Unrecognized command: " << input_char << std::endl;
                }
                break;
            }
                
            case 'e':{
                if(can_edit){
                    next_screen = context::Screen::EDIT_POST;
                }else{
                    std::cout << "Unrecognized command: " << input_char << std::endl;
                }
                break;
            }
            case 'r':{
                if(can_edit){
                    next_screen = context::Screen::REMOVE_POST;
                }else{
                    std::cout << "Unrecognized command: " << input_char << std::endl;
                }
                break;
            }
                
            case 'l':{
                next_screen = context::Screen::LIKE_POST;
                break;
            }
            case 'u':{
                next_screen = context::Screen::UNLIKE_POST;
                break;
            }
                
            case 'a':{
                next_screen = context::Screen::ADD_COMMENT;
                break;
            }
            case 'v':{
                next_screen = context::Screen::VIEW_NEXT_COMMENTS;
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
    
    const context::ScreenHandler handleEditPost = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::VIEW_NEXT_POST;
        
        std::cout << "Edit Post [C]ontent" << std::endl;
        std::cout << "Edit Post [P]ermission" << std::endl;
        std::cout << context::PROMPT_CHAR;
        
        char input_char;
        std::cin >> input_char;
        if(std::tolower(input_char) == 'c'){
            
            std::cout << "Enter Content: " ;
            std::string content("");
            std::cin.ignore();
            std::getline(std::cin, content);
            
            flatbuffers::FlatBufferBuilder builder_edit, builder_req;
            auto session = fbs::CreateSession(builder_edit,
                                              builder_edit.CreateString(context::CurrentTokenStr));
            auto edit_req = fbs::post::CreateEditPostRequest(builder_edit,
                                                             session,
                                                             (uint64_t)context::post::CurrentPid,
                                                             builder_edit.CreateString(content));
            fbs::post::FinishEditPostRequestBuffer(builder_edit, edit_req);
            
            utils::BuildRequest("/post/edit", builder_req, builder_edit);
            
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
                                         }
                                         
                                         std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                     });
            
        }else if(std::tolower(input_char) == 'p'){
            
            std::cout << "[1] Only You" << std::endl;
            std::cout << "[2] Public" << std::endl;
            std::cout << "[3] Friends" << std::endl;
            std::cout << "[4] Specific Users" << std::endl;
            std::cout << context::PROMPT_CHAR;
            
            std::cin >> input_char;
            auto perm_mask = fbs::post::PostPermissionType_ANY;
            switch(input_char){
                case '1':{
                    perm_mask = fbs::post::PostPermissionType_PRIVATE;
                    break;
                }
                    
                case '2':{
                    perm_mask = fbs::post::PostPermissionType_ANY;
                    break;
                }
                
                case '3':{
                    perm_mask = fbs::post::PostPermissionType_FRIENDS;
                    break;
                }
                    
                case '4':{
                    perm_mask = fbs::post::PostPermissionType_POSTER_DEFINED;
                    break;
                }
                    
                default:{
                    std::cout << "Unrecognized comment: " << input_char << std::endl;
                    return context::Screen::STAY;
                }
            }
            
            flatbuffers::FlatBufferBuilder builder_edit, builder_req;
            
            std::vector< flatbuffers::Offset<flatbuffers::String> > dummy_perms;
            
            if(perm_mask == fbs::post::PostPermissionType_POSTER_DEFINED){
                std::cout << "Enter users that can view(separated by comma): " << std::endl;
                std::cout << "->" ;
                std::cin.ignore();
                std::string users("");
                std::getline(std::cin, users);
                
                utils::TrimString(users, ',');
                boost::char_separator<char> separator(", ");
                boost::tokenizer< boost::char_separator<char> > tokens(users, separator);
                for(auto token : tokens){
                    dummy_perms.push_back(builder_edit.CreateString(token));
                }
            }
            
            auto perm = fbs::post::CreatePostPermission(builder_edit,
                                                        perm_mask,
                                                        builder_edit.CreateVector(dummy_perms));
            
            auto session = fbs::CreateSession(builder_edit,
                                              builder_edit.CreateString(context::CurrentTokenStr));
            auto edit_req = fbs::post::CreateEditPostPermissionRequest(builder_edit,
                                                                       session,
                                                                       (uint64_t)context::post::CurrentPid,
                                                                       perm);
            fbs::post::FinishEditPostPermissionRequestBuffer(builder_edit, edit_req);
            
            utils::BuildRequest("/post/edit/perm", builder_req, builder_edit);
            
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
                                         }
                                         
                                         std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                     });
            
        }else{
            std::cout << "Unrecognized comment: " << input_char << std::endl;
            next_screen = context::Screen::STAY;
        }
        
        return next_screen;
    };
    
    const context::ScreenHandler handleRemovePost = SCREEN_HANDLER(){
        
        auto next_screen = context::Screen::VIEW_NEXT_POST;
        
        std::cout << "Are you sure? [Y/n]: ";
        char input_char;
        std::cin >> input_char;
        
        if(std::tolower(input_char) == 'y'){
            flatbuffers::FlatBufferBuilder builder_remove, builder_req;
            
            auto session = fbs::CreateSession(builder_remove,
                                              builder_remove.CreateString(context::CurrentTokenStr));
            auto remove_req = fbs::post::CreateRemovePostRequest(builder_remove,
                                                                 session,
                                                                 (uint64_t)context::post::CurrentPid);
            fbs::post::FinishRemovePostRequestBuffer(builder_remove, remove_req);
            
            utils::BuildRequest("/post/remove", builder_req, builder_remove);
            
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
                                             //Reset post info
                                             context::post::MaxPid = context::post::CurrentPid = -1;
                                         }
                                         
                                         std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                     });
        }
        
        return next_screen;
    };
    
    const context::ScreenHandler handleAddComment = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::VIEW_NEXT_POST;
        
        std::string comment_str("");
        std::cout << "Enter Comment: ";
        std::cin.ignore();
        std::getline(std::cin, comment_str);
        
        flatbuffers::FlatBufferBuilder builder_comment, builder_req;
        
        auto session = fbs::CreateSession(builder_comment,
                                          builder_comment.CreateString(context::CurrentTokenStr));
        auto comment_req=  fbs::post::CreateNewCommentRequest(builder_comment,
                                                              session,
                                                              context::post::CurrentPid,
                                                              builder_comment.CreateString(comment_str));
        fbs::post::FinishNewCommentRequestBuffer(builder_comment, comment_req);
        
        utils::BuildRequest("/post/comment/add", builder_req, builder_comment);
        
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
                                     }
                                     
                                     std::cout << utils::GetErrorVerbose(resp->status_code()) << std::endl;
                                 });
        
        return next_screen;
    };
    
    const context::ScreenHandler handleViewComment = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::VIEW_NEXT_POST;
        
        //Get Post
        flatbuffers::FlatBufferBuilder builder_comment, builder_req;
        
        auto session_fbs = fbs::CreateSession(builder_comment,
                                              builder_comment.CreateString(context::CurrentTokenStr));
        
        auto view_comment_req = fbs::post::CreateGetCommentRequest(builder_comment,
                                                                   session_fbs,
                                                                   (uint64_t)context::post::CurrentPid,
                                                                   (uint64_t)context::post::CurrentCid);
        fbs::post::FinishGetCommentRequestBuffer(builder_comment, view_comment_req);
        
        utils::BuildRequest("/post/comment/view", builder_req, builder_comment);
        
        bool skip = false;
        utils::ClientSendAndRead(context::SocketFd, builder_req, [&](char* buffer, ssize_t n_bytes)->void{
            //Parsing response
            if(n_bytes < 0){
                std::cout << "Communication Error" << std::endl;
                next_screen = context::Screen::MAIN;
                return;
            }
            
            auto* resp = fbs::post::GetGetCommentResponse(buffer);
            auto status_code = resp->status_code();
            if(status_code != fbs::Status_OK){
                skip = true;
            }else{
                std::cout << resp->commenter()->str() << ": " << resp->content()->str() << std::endl;
            }
        });
        
        if(skip){
            switch(current_screen){
                case context::Screen::VIEW_NEXT_COMMENTS:{
                    if(context::post::CurrentCid < context::post::MaxCid){
                        context::post::CurrentCid++;
                        return context::Screen::STAY;
                    }else{
                        return context::Screen::MAIN;
                    }
                }
                    
                case context::Screen::VIEW_PREV_COMMENTS:{
                    if(context::post::CurrentCid > 1){
                        context::post::CurrentCid--;
                        return context::Screen::STAY;
                    }else{
                        return context::Screen::MAIN;
                    }
                }
                    
                default:
                    break;
            }
        }
        
        std::cout << std::endl;
        if(context::post::CurrentCid > 1){
            std::cout << "[P]revious Comment\t";
        }
        if(context::post::CurrentCid < context::post::MaxCid){
            std::cout << "[N]ext Comment";
        }
        
        std::cout <<std::endl;
        std::cout << "[B]ack" << std::endl;
        
        std::cout << context::PROMPT_CHAR;
        char input_char;
        std::cin >> input_char;
        
        switch(std::tolower(input_char)){
            case 'n':{
                if(context::post::CurrentCid < context::post::MaxCid){
                    context::post::CurrentCid++;
                    next_screen = context::Screen::VIEW_NEXT_COMMENTS;
                }else{
                    std::cout << "Unrecognized command: " << input_char << std::endl;
                }
                break;
            }
                
            case 'p':{
                if(context::post::CurrentCid > 1){
                    context::post::CurrentCid--;
                    next_screen = context::Screen::VIEW_PREV_COMMENTS;
                }else{
                    std::cout << "Unrecognized command: " << input_char << std::endl;
                }
                break;
            }
                
            case 'b':{
                next_screen = context::Screen::VIEW_NEXT_POST;
                break;
            }
                
            default:{
                std::cout << "Unrecognized command: " << input_char << std::endl;
                next_screen = context::Screen::STAY;
                break;
            }
        }
        
        return next_screen;
    };
    
    const context::ScreenHandler handleLike = SCREEN_HANDLER(){
        
        context::PrintDivideLine();
        
        auto next_screen = context::Screen::VIEW_NEXT_POST;
        
        flatbuffers::FlatBufferBuilder builder_like, builder_req;
        
        int like_num = 0;
        if(current_screen == context::Screen::LIKE_POST){
            like_num = 1;
        }else if(current_screen == context::Screen::UNLIKE_POST){
            like_num = -1;
        }else{
            return next_screen;
        }
        
        auto session = fbs::CreateSession(builder_like,
                                          builder_like.CreateString(context::CurrentTokenStr));
        auto like_req = fbs::post::CreateLikeRequest(builder_like,
                                                     session,
                                                     (int)context::post::CurrentPid,
                                                     (int)like_num);
        fbs::post::FinishLikeRequestBuffer(builder_like, like_req);
        
        utils::BuildRequest("/post/like", builder_req, builder_like);
        
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
    
    void InitScreens(){
        context::AddScreen(context::Screen::ADD_POST, handleAddPost);
        context::AddScreen(context::Screen::EDIT_POST, handleEditPost);
        context::AddScreen(context::Screen::VIEW_NEXT_POST, handleViewPost);
        context::AddScreen(context::Screen::VIEW_PREV_POST, handleViewPost);
        context::AddScreen(context::Screen::REMOVE_POST, handleRemovePost);
        context::AddScreen(context::Screen::ADD_COMMENT, handleAddComment);
        context::AddScreen(context::Screen::VIEW_PREV_COMMENTS, handleViewComment);
        context::AddScreen(context::Screen::VIEW_NEXT_COMMENTS, handleViewComment);
        context::AddScreen(context::Screen::LIKE_POST, handleLike);
        context::AddScreen(context::Screen::UNLIKE_POST, handleLike);
    }
    
} //namespace post