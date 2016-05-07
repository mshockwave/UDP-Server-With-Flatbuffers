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
        const char* PERM_MASK_KEY = "permission/mask"; //number(bits mask)
        const char* PERM_GRANTED_KEY = "permission/granted"; //string array
        const char* POSTER_ADDR_KEY = "addr"; //string
        const char* CONTENT_KEY = "content";
        const char* TIMESTAMP_KEY = "timestamp"; //string
        const char* COMMENTS_KEY = "comments"; //array of comment
        const char* LIKES_NUM_KEY = "likes"; //number
        
        void loadPostFile(){
            using namespace boost::property_tree;
            try{
                Posts.clear();
                read_json(POST_FILE_NAME, Posts);
            }catch(const json_parser_error& e){
                Log::E("Post File Loader") << "Failed loading post file: " << e.message() << std::endl;
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
        
        const HandleFunc handleEditPost = HANDLE_FUNC(){
            
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
        .Path("/edit", post::handleEditPost);
    }
    
} //namespace handlers