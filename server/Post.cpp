#include <ctime>

#include "Post.hpp"
#include "Session.hpp"

#include <Utils.hpp>
#include <schemas/post.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

static boost::property_tree::ptree Posts;

namespace handlers {
    
    namespace post{
        
        const char* LAST_POST_ID = "lastPostId";
        const char* POSTER_NAME_KEY = "account";
        const char* POSTER_NICKNAME_KEY = "nickname";
        const char* PERM_TREE_KEY = "permission";
        const char* PERM_MASK_KEY = "permission/mask"; //number(bits mask)
        const char* PERM_GRANTED_KEY = "permission/granted"; //string array
        const char* PERM_GRANTED_NODE_KEY = "granted";
        const char* POSTER_ADDR_KEY = "addr"; //string
        const char* CONTENT_KEY = "content";
        const char* TIMESTAMP_KEY = "timestamp"; //string
        const char* LIKES_NUM_KEY = "likes"; //number
        const char* COMMENT_TREE_KEY = "comments"; //array of comment
        /*
         comments: {
            0:{
                user: "username",
                content: "This is content"
            },
            ...
            lastCommentId: N
         }
         */
        const char* COMMENT_MAX_COMMENT_ID = "lastCommentId";
        const char* COMMENT_USER_KEY = "user";
        const char* COMMENT_CONTENT_KEY = "content";
        
        void loadPostFile(){
            using namespace boost::property_tree;
            try{
                Posts.clear();
                read_json(POST_FILE_NAME, Posts);
            }catch(const json_parser_error& e){
                Log::E("Post File Loader") << "Failed loading post file: " << e.message() << std::endl;
            }
        }
        
        bool canViewPost(const boost::property_tree::ptree& post_tree,
                         const std::string& username){
            try{
                
                std::string poster = post_tree.get(GetPath(POSTER_NAME_KEY), "");
                if(poster.length() <= 0) return false;
                if(poster == username) return true;
                
                int perm_mask = post_tree.get<int>(GetPath(PERM_MASK_KEY),
                                                   fbs::post::PostPermissionType_ANY);
                
                if( (perm_mask & (int)fbs::post::PostPermissionType_ANY) != 0){
                    return true;
                }
                
                if( (perm_mask & (int)fbs::post::PostPermissionType_POSTER_DEFINED) != 0){
                    auto& granted_tree = post_tree.get_child(GetPath(PERM_TREE_KEY));
                    auto it_granted = granted_tree.begin();
                    for(; it_granted != granted_tree.end(); ++it_granted){
                        if(it_granted->first == PERM_GRANTED_NODE_KEY){
                            const std::string& name = (it_granted->second).data();
                            if(username == name) return true;
                        }
                    }
                }
                
                return false;
            }catch(const boost::property_tree::ptree_bad_path&){
                return false;
            }
        }
        
        const HandleFunc handleNewPost = HANDLE_FUNC(){
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::post::VerifyNewPostRequestBuffer(verifier)){
                
                auto* post_req = fbs::post::GetNewPostRequest(request.payload()->Data());
                const auto* session = post_req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        auto nickname = utils::account::GetNickName(username);
                        
                        const auto* client_ip = request.GetClientAddrStr();
                        auto client_port = request.GetClientPort();
                        std::string client_addr(client_ip);
                        client_addr += ":";
                        client_addr += std::to_string(client_port);
                        
                        long last_post_id = Posts.get(GetPath(LAST_POST_ID), 0L);
                        auto post_id = last_post_id + 1L;
                        Posts.put(GetPath(LAST_POST_ID), post_id);
                        
                        const auto content_str = post_req->content()->str();
                        auto perm_mask = post_req->permission()->type();
                        const auto* perm_user_list = post_req->permission()->user_list();
                        
                        using namespace boost::property_tree;
                        ptree post_tree;
                        post_tree.put(GetPath(POSTER_NAME_KEY), username);
                        post_tree.put(GetPath(POSTER_NICKNAME_KEY), nickname);
                        post_tree.put(GetPath(PERM_MASK_KEY), (int)perm_mask);
                        auto it_user = perm_user_list->begin();
                        for(; it_user != perm_user_list->end(); ++it_user){
                            post_tree.add(GetPath(PERM_GRANTED_KEY), it_user->str());
                        }
                        
                        post_tree.put(GetPath(CONTENT_KEY), content_str);
                        post_tree.put(GetPath(POSTER_ADDR_KEY), client_addr);
                        time_t raw_time;
                        time(&raw_time);
                        auto* time_info = localtime(&raw_time);
                        std::string time_str(asctime(time_info));
                        post_tree.put(GetPath(TIMESTAMP_KEY), time_str);
                        
                        Posts.put_child(GetPath(std::to_string(post_id)), post_tree);
                        
                        //TODO: Update session token
                        SendStatusResponse(fbs::Status_OK, response_writer);
                    }catch(const session::BadTransformException&){
                        SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                    }
                    
                }else{
                    SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                }
                
            }else{
                Log::W("New Post Handler") << "Payload format invalid" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
        
        const HandleFunc handleGetMaxPid = HANDLE_FUNC(){
            
            const auto send_status = [&response_writer](fbs::Status status)->void{
                flatbuffers::FlatBufferBuilder builder;
                auto resp = fbs::post::CreateGetMaxPidResponse(builder,
                                                               status);
                fbs::post::FinishGetMaxPidResponseBuffer(builder, resp);
                response_writer(builder.GetBufferPointer(), builder.GetSize());
            };
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::post::VerifyGetMaxPidRequestBuffer(verifier)){
                
                auto* req = fbs::post::GetGetMaxPidRequest(request.payload()->Data());
                const auto* session = req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    long max_post_id = Posts.get(GetPath(LAST_POST_ID), 0L);
                    
                    flatbuffers::FlatBufferBuilder builder;
                    auto resp = fbs::post::CreateGetMaxPidResponse(builder,
                                                                   fbs::Status_OK,
                                                                   0,
                                                                   static_cast<uint64_t>(max_post_id));
                    fbs::post::FinishGetMaxPidResponseBuffer(builder, resp);
                    response_writer(builder.GetBufferPointer(), builder.GetSize());
                }else{
                    send_status(fbs::Status_AUTH_ERROR);
                }
                
            }else{
                Log::W("Get Max Pid Handler") << "Payload format invalid" << std::endl;
                send_status(fbs::Status_PAYLOAD_FORMAT_INVALID);
            }
        };
        
        const HandleFunc handleViewPost = HANDLE_FUNC(){
            
            const auto send_status = [&response_writer](fbs::Status status)->void{
                flatbuffers::FlatBufferBuilder builder;
                auto resp = fbs::post::CreateGetPostResponse(builder,
                                                             status);
                fbs::post::FinishGetPostResponseBuffer(builder, resp);
                response_writer(builder.GetBufferPointer(), builder.GetSize());
            };
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::post::VerifyGetPostRequestBuffer(verifier)){
                
                auto* view_req = fbs::post::GetGetPostRequest(request.payload()->Data());
                const auto* session = view_req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    using namespace boost::property_tree;
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        auto nickname = utils::account::GetNickName(username);
                        auto post_id = view_req->post_id();
                        
                        ptree& post_tree = Posts.get_child(GetPath(std::to_string(post_id)));
                        if(!canViewPost(post_tree, username)){
                            send_status(fbs::Status_PERMISSION_DENIED);
                            return;
                        }
                        
                        auto poster_name = post_tree.get(GetPath(POSTER_NAME_KEY), "");
                        auto poster_nickname = post_tree.get(GetPath(POSTER_NICKNAME_KEY), "");
                        auto addr_str = post_tree.get(GetPath(POSTER_ADDR_KEY), "");
                        auto content = post_tree.get(GetPath(CONTENT_KEY), "");
                        auto post_time_str = post_tree.get(GetPath(TIMESTAMP_KEY), "");
                        utils::TrimString(post_time_str, '\n');
                        int like_num = post_tree.get<int>(GetPath(LIKES_NUM_KEY), 0);
                        
                        //Retrieve max comment id
                        auto max_cid_path = utils::JoinPath({COMMENT_TREE_KEY, COMMENT_MAX_COMMENT_ID});
                        int max_cid = post_tree.get<int>(GetPath(max_cid_path), 0);
                        
                        flatbuffers::FlatBufferBuilder builder;
                        auto view_resp = fbs::post::CreateGetPostResponse(builder,
                                                                          fbs::Status_OK, 0,
                                                                          builder.CreateString(poster_name),
                                                                          builder.CreateString(poster_nickname),
                                                                          builder.CreateString(addr_str),
                                                                          builder.CreateString(content),
                                                                          builder.CreateString(post_time_str),
                                                                          like_num,
                                                                          (uint64_t)max_cid);
                        fbs::post::FinishGetPostResponseBuffer(builder, view_resp);
                        response_writer(builder.GetBufferPointer(), builder.GetSize());
                    }catch(const session::BadTransformException&){
                        send_status(fbs::Status_AUTH_ERROR);
                    }catch(const ptree_bad_path&){
                        send_status(fbs::Status_INVALID_REQUEST_ARGUMENT);
                    }
                    
                }else{
                    send_status(fbs::Status_AUTH_ERROR);
                }
            }else{
                Log::W("View Post Handler") << "Payload format invalid" << std::endl;
                send_status(fbs::Status_PAYLOAD_FORMAT_INVALID);
            }
        };
        
        const HandleFunc handleLike = HANDLE_FUNC(){
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::post::VerifyLikeRequestBuffer(verifier)){
                
                auto* like_req = fbs::post::GetLikeRequest(request.payload()->Data());
                const auto* session = like_req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    using namespace boost::property_tree;
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        
                        auto post_id = like_req->post_id();
                        auto like_num = like_req->like_num();
                        ptree& post_tree = Posts.get_child(GetPath(std::to_string(post_id)));
                        
                        like_num += post_tree.get<int>(GetPath(LIKES_NUM_KEY), 0);
                        
                        post_tree.put(GetPath(LIKES_NUM_KEY), like_num);
                        
                        SendStatusResponse(fbs::Status_OK, response_writer);
                        
                    }catch(const ptree_bad_path&){
                        SendStatusResponse(fbs::Status_INVALID_REQUEST_ARGUMENT, response_writer);
                    }catch(const session::BadTransformException&){
                        SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                    }
                    
                }else{
                    SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                }
                
            }else{
                Log::W("Like Handler") << "Payload format invalid" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
        
        const HandleFunc handleEditPost = HANDLE_FUNC(){
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::post::VerifyEditPostRequestBuffer(verifier)){
                
                auto* edit_req = fbs::post::GetEditPostRequest(request.payload()->Data());
                const auto* session = edit_req->session();
                
                if(session::IsSessionExist(*session)){
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        
                        auto post_id_str = std::to_string(edit_req->post_id());
                        
                        boost::property_tree::ptree& post_tree = Posts.get_child(GetPath(post_id_str));
                        
                        //Check whether is poster
                        auto poster_name = post_tree.get(GetPath(POSTER_NAME_KEY), "");
                        if(poster_name.length() <= 0 || poster_name != username){
                            SendStatusResponse(fbs::Status_PERMISSION_DENIED, response_writer);
                            return;
                        }
                        
                        auto req_content = edit_req->content()->str();
                        
                        post_tree.put(GetPath(CONTENT_KEY), req_content);
                        
                        SendStatusResponse(fbs::Status_OK, response_writer);
                    }catch(const session::BadTransformException&){
                        SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                    }catch(const boost::property_tree::ptree_bad_path&){
                        SendStatusResponse(fbs::Status_INVALID_REQUEST_ARGUMENT, response_writer);
                    }
                }else{
                    SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                }
            }else{
                Log::W("Edit Post Handler") << "Payload format invalid" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
        
        const HandleFunc handleEditPostPerm = HANDLE_FUNC(){
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::post::VerifyEditPostPermissionRequestBuffer(verifier)){
                
                auto* edit_req = fbs::post::GetEditPostPermissionRequest(request.payload()->Data());
                const auto* session = edit_req->session();
                
                if(session::IsSessionExist(*session)){
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        
                        auto post_id_str = std::to_string(edit_req->post_id());
                        
                        boost::property_tree::ptree& post_tree = Posts.get_child(GetPath(post_id_str));
                        
                        auto perm_mask = edit_req->permission()->type();
                        const auto* perm_user_list = edit_req->permission()->user_list();
                        
                        post_tree.put(GetPath(PERM_MASK_KEY), (int)perm_mask);
                        auto it_user = perm_user_list->begin();
                        for(; it_user != perm_user_list->end(); ++it_user){
                            if(it_user == perm_user_list->begin()){
                                //Replace on first
                                post_tree.put(GetPath(PERM_GRANTED_KEY), it_user->str());
                            }else{
                                post_tree.add(GetPath(PERM_GRANTED_KEY), it_user->str());
                            }
                        }
                        
                        SendStatusResponse(fbs::Status_OK, response_writer);
                    }catch(const session::BadTransformException&){
                        SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                    }catch(const boost::property_tree::ptree_bad_path&){
                        SendStatusResponse(fbs::Status_INVALID_REQUEST_ARGUMENT, response_writer);
                    }
                }else{
                    SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                }
            }else{
                Log::W("Edit Post Perm Handler") << "Payload format invalid" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
        
        const HandleFunc handleAddComment = HANDLE_FUNC(){
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::post::VerifyNewCommentRequestBuffer(verifier)){
                
                auto* new_comment_req = fbs::post::GetNewCommentRequest(request.payload()->Data());
                const auto* session = new_comment_req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    using namespace boost::property_tree;
                    
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        auto post_id = new_comment_req->post_id();
                        
                        ptree& post_tree = Posts.get_child(GetPath(std::to_string(post_id)));
                        if(!canViewPost(post_tree, username)){
                            SendStatusResponse(fbs::Status_PERMISSION_DENIED, response_writer);
                            return;
                        }
                        
                        auto max_cid_path = utils::JoinPath({COMMENT_TREE_KEY, COMMENT_MAX_COMMENT_ID});
                        int cid = post_tree.get<int>(GetPath(max_cid_path), 0) + 1;
                        auto content = new_comment_req->content()->str();
                        
                        ptree comment_tree;
                        comment_tree.put(GetPath(COMMENT_USER_KEY), username);
                        comment_tree.put(GetPath(COMMENT_CONTENT_KEY), content);
                        
                        post_tree.put(GetPath(max_cid_path), cid);
                        auto new_comment_path = utils::JoinPath({COMMENT_TREE_KEY, std::to_string(cid)});
                        post_tree.put_child(GetPath(new_comment_path), comment_tree);
                        
                        SendStatusResponse(fbs::Status_OK, response_writer);
                    }catch(const ptree_bad_path&){
                        SendStatusResponse(fbs::Status_INVALID_REQUEST_ARGUMENT, response_writer);
                    }catch(const session::BadTransformException&){
                        SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                    }
                    
                }else{
                    SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                }
                
            }else{
                Log::W("Add Comment Handler") << "Payload format invalid" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
        
        const HandleFunc handleViewComment = HANDLE_FUNC(){
            
            const auto send_status = [&response_writer](fbs::Status status)->void{
                flatbuffers::FlatBufferBuilder builder;
                auto resp = fbs::post::CreateGetCommentResponse(builder,
                                                                status);
                fbs::post::FinishGetCommentResponseBuffer(builder, resp);
                response_writer(builder.GetBufferPointer(), builder.GetSize());
            };
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::post::VerifyGetCommentRequestBuffer(verifier)){
                
                auto* view_req = fbs::post::GetGetCommentRequest(request.payload()->Data());
                const auto* session = view_req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    using namespace boost::property_tree;
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        auto post_id = view_req->post_id();
                        
                        ptree& post_tree = Posts.get_child(GetPath(std::to_string(post_id)));
                        if(!canViewPost(post_tree, username)){
                            send_status(fbs::Status_PERMISSION_DENIED);
                            return;
                        }
                        
                        auto comment_id = view_req->comment_id();
                        
                        auto comment_tree_path = utils::JoinPath({COMMENT_TREE_KEY, std::to_string(comment_id)});
                        ptree& comment_tree = post_tree.get_child(GetPath(comment_tree_path));
                        
                        auto comment_user = comment_tree.get(GetPath(COMMENT_USER_KEY), "");
                        auto comment_content = comment_tree.get(GetPath(COMMENT_CONTENT_KEY), "");
                        
                        //Build response
                        flatbuffers::FlatBufferBuilder builder;
                        auto resp = fbs::post::CreateGetCommentResponse(builder,
                                                                        fbs::Status_OK, 0,
                                                                        builder.CreateString(comment_user),
                                                                        builder.CreateString(comment_content));
                        fbs::post::FinishGetCommentResponseBuffer(builder, resp);
                        
                        response_writer(builder.GetBufferPointer(), builder.GetSize());
                        
                    }catch(const ptree_bad_path&){
                        send_status(fbs::Status_INVALID_REQUEST_ARGUMENT);
                    }catch(const session::BadTransformException&){
                        send_status(fbs::Status_AUTH_ERROR);
                    }
                    
                }else{
                    send_status(fbs::Status_AUTH_ERROR);
                }
                
            }else{
                Log::W("View Comment Handler") << "Payload format invalid" << std::endl;
                send_status(fbs::Status_PAYLOAD_FORMAT_INVALID);
            }
        };
        
    } //namespace post
    
    void InitPostHandlers(Router& router){
        
        post::loadPostFile();
        //Schedule write back task
        utils::AddFinalizeCallback([&]{
            //Write account data back to file
            using namespace boost::property_tree;
            try{
                write_json(POST_FILE_NAME, Posts);
            }catch(const json_parser_error& e){
                Log::E("Post File Loader") << "Failed writing back post file: " << e.message() << std::endl;
            }
        });
        
        router.Path("/new", post::handleNewPost)
        .Path("/edit", post::handleEditPost)
        .Path("/edit/perm", post::handleEditPostPerm)
        .Path("/view", post::handleViewPost)
        .Path("/view/maxPid", post::handleGetMaxPid)
        .Path("/like", post::handleLike)
        .Path("/comment/add", post::handleAddComment)
        .Path("/comment/view", post::handleViewComment);
    }
    
} //namespace handlers