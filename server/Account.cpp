
#include "Account.hpp"
#include "Session.hpp"
#include <Utils.hpp>
#include <schemas/account.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

static boost::property_tree::ptree AccountProfiles;

static std::unordered_map<std::string, std::unordered_set<std::string> > PendingFriendRequests;

namespace utils{
    namespace account{
        std::string GetNickName(const std::string& username){
            std::string path(username);
            path += PATH_SEPARATOR;
            path += PROFILE_NIKNAME_KEY;
            return AccountProfiles.get(GetPath(path), "");
        }
    } //namespace account
} //namespace utils

namespace handlers{
    
    namespace account {
        
        void loadAccountFile(){
            using namespace boost::property_tree;
            try{
                AccountProfiles.clear();
                read_json(ACCOUNT_FILE_NAME, AccountProfiles);
            }catch(const json_parser_error& e){
                Log::E("Account File Loader") << "Failed loading account file: " << e.message() << std::endl;
            }
        }
        
        const HandleFunc handleRegister = HANDLE_FUNC(){
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::account::VerifyRegisterRequestBuffer(verifier)){
                auto* register_req = fbs::account::GetRegisterRequest(request.payload()->Data());
                
                auto username = register_req->username()->str();
                auto password = register_req->password()->str();
                
                if(username.length() <= 0 || password.length() <= 0){
                    SendStatusResponse(fbs::Status_REGISTER_INFO_INVALID, response_writer);
                    return;
                }
                
                using namespace boost::property_tree;
                //Check account existance
                bool exist = true;
                try{
                    AccountProfiles.get_child(GetPath(username));
                }catch(const ptree_bad_path&){
                    exist = false;
                }
                if(exist){
                    SendStatusResponse(fbs::Status_USER_EXIST, response_writer);
                    return;
                }
                
                ptree user_tree;
                user_tree.put(GetPath(PROFILE_NIKNAME_KEY), username);
                user_tree.put(GetPath(PROFILE_PASSWORD_KEY), password);
                
                AccountProfiles.put_child(GetPath(username), user_tree);
                
                {
                    //Send response
                    flatbuffers::FlatBufferBuilder builder;
                    auto resp = fbs::CreateGeneralResponse(builder,
                                                           session::NewSession(builder, username,
                                                                               request.GetRawSockAddr()),
                                                           fbs::Status_OK);
                    fbs::FinishGeneralResponseBuffer(builder, resp);
                    
                    response_writer(builder.GetBufferPointer(), builder.GetSize());
                }
            }else{
                Log::W("Account Register Handler") << "Payload format invalid" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
        
        const HandleFunc handleLogin = HANDLE_FUNC(){
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::account::VerifyLoginRequestBuffer(verifier)){
                
                auto* login_req = fbs::account::GetLoginRequest(request.payload()->Data());
                
                const auto& username = login_req->username()->str();
                const auto& password = login_req->password()->str();
                
                using namespace boost::property_tree;
                try{
                    auto& account_tree = AccountProfiles.get_child(GetPath(username));
                    
                    const auto& password_correct = account_tree.get(GetPath(PROFILE_PASSWORD_KEY), "");
                    if(password_correct == "" ||
                       password_correct != password){
                        SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                        return;
                    }
                    
                    //Generate new session
                    {
                        flatbuffers::FlatBufferBuilder builder;
                        auto resp = fbs::CreateGeneralResponse(builder,
                                                               session::NewSession(builder, username,
                                                                                   request.GetRawSockAddr()),
                                                               fbs::Status_OK);
                        fbs::FinishGeneralResponseBuffer(builder, resp);
                        
                        response_writer(builder.GetBufferPointer(), builder.GetSize());
                    }
                }catch(const ptree_bad_path&){
                    SendStatusResponse(fbs::Status_AUTH_ERROR, response_writer);
                }
            }else{
                Log::W("Account Login Handler") << "Payload format invalid" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
        
        const HandleFunc handleLogout = HANDLE_FUNC(){
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::account::VerifyLogoutRequestBuffer(verifier)){
                
                auto* logout_req = fbs::account::GetLogoutRequest(request.payload()->Data());
                
                auto* session = logout_req->session();
                
                try{
                    const auto& username = session::GetStringValue(*session,
                                                                   session::SESSION_KEY_USERNAME);
                    Log::V("Logout handler") << username << " logout" << std::endl;
                    
                    session::RemoveSession(*session);
                    
                    /*
                    Log::D("Logout handler") << "Is session exist: " <<
                                                session::IsSessionExist(*session) << std::endl;
                     */
                    {
                        //Send logout response
                        flatbuffers::FlatBufferBuilder builder;
                        auto general_resp = fbs::CreateGeneralResponse(builder, 0,
                                                                       fbs::Status_OK);
                        fbs::FinishGeneralResponseBuffer(builder, general_resp);
                        
                        response_writer(builder.GetBufferPointer(), builder.GetSize());
                    }
                }catch(const session::BadTransformException &e){
                    Log::W("Logout handler") << "Logout fail: " << e.cause <<std::endl;
                    {
                        //Send logout response
                        flatbuffers::FlatBufferBuilder builder;
                        auto general_resp = fbs::CreateGeneralResponse(builder, 0,
                                                                       fbs::Status_UNKNOWN_ERROR);
                        fbs::FinishGeneralResponseBuffer(builder, general_resp);
                        
                        response_writer(builder.GetBufferPointer(), builder.GetSize());
                    }
                }
            }else{
                Log::W("Account Logout Handler") << "Payload format invalid" << std::endl;
                {
                    //Send logout response
                    flatbuffers::FlatBufferBuilder builder;
                    auto general_resp = fbs::CreateGeneralResponse(builder, 0,
                                                                   fbs::Status_PAYLOAD_FORMAT_INVALID);
                    fbs::FinishGeneralResponseBuffer(builder, general_resp);
                    
                    response_writer(builder.GetBufferPointer(), builder.GetSize());
                }
            }
        };
        
        const HandleFunc handleSearchAccount = HANDLE_FUNC(){
            
            const auto send_status = [&response_writer](fbs::Status status)->void{
                flatbuffers::FlatBufferBuilder builder;
                auto resp = fbs::account::CreateSearchResponse(builder,
                                                               status);
                fbs::account::FinishSearchResponseBuffer(builder, resp);
                response_writer(builder.GetBufferPointer(), builder.GetSize());
            };
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::account::VerifySearchRequestBuffer(verifier)){
                
                auto* search_req = fbs::account::GetSearchRequest(request.payload()->Data());
                const auto* session = search_req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    using namespace boost::property_tree;
                    
                    try{
                        
                        const auto& query_str = search_req->query_str()->str();
                        flatbuffers::FlatBufferBuilder builder;
                        std::vector< flatbuffers::Offset<flatbuffers::String> > results;
                        
                        for(auto it_account : AccountProfiles){
                            if(!it_account.second.empty()){
                                auto username = it_account.first;
                                auto& account_tree = it_account.second;
                                auto nickname = account_tree.get(GetPath(PROFILE_NIKNAME_KEY), "");
                                
                                std::stringstream ss;
                                ss << username << " (" << nickname << ")";
                                
                                if(username.find(query_str) != std::string::npos){
                                    results.push_back(builder.CreateString(ss.str()));
                                }else if(nickname.size() != 0 &&
                                         nickname.find(query_str) != std::string::npos){
                                    results.push_back(builder.CreateString(ss.str()));
                                }
                            }
                        }
                        
                        auto resp = fbs::account::CreateSearchResponse(builder,
                                                                       fbs::Status_OK, 0,
                                                                       builder.CreateVector(results));
                        fbs::account::FinishSearchResponseBuffer(builder, resp);
                        
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
                Log::W("Account Search Handler") << "Payload format error" << std::endl;
                send_status(fbs::Status_PAYLOAD_FORMAT_INVALID);
            }
        };
        
        const HandleFunc handleAddFriends = HANDLE_FUNC(){
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::account::VerifyAddFriendRequestBuffer(verifier)){
                
                auto* req = fbs::account::GetAddFriendRequest(request.payload()->Data());
                const auto* session = req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    using namespace boost::property_tree;
                    
                    const auto& req_friend_name = req->username()->str();
                    
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        
                        //Ensure that account exist
                        AccountProfiles.get_child(GetPath(req_friend_name));
                        
                        //Check whether the guy had sent request before
                        //If yes, this call would be friendship confirmation
                        auto& my_set = PendingFriendRequests[username];
                        auto it_result = my_set.find(req_friend_name);
                        if( it_result != my_set.end()){
                            //Friendship confirmation
                            
                            ptree& my_account_tree = AccountProfiles.get_child(GetPath(username));
                            ptree& his_account_tree = AccountProfiles.get_child(GetPath(req_friend_name));
                            
                            auto my_path = utils::JoinPath({PROFILE_FRIENDS_KEY, req_friend_name});
                            auto his_path = utils::JoinPath({PROFILE_FRIENDS_KEY, username});
                            
                            my_account_tree.add(GetPath(my_path), "placeholder"/*Not empty string*/);
                            his_account_tree.add(GetPath(his_path), "placeholder");
                            
                            my_set.erase(it_result);
                        }else{
                            //New request
                            auto& pending_set = PendingFriendRequests[req_friend_name];
                            pending_set.insert(username);
                        }
                        
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
                Log::W("Add Friend Handler") << "Payload format error" << std::endl;
                SendStatusResponse(fbs::Status_PAYLOAD_FORMAT_INVALID, response_writer);
            }
        };
        
        const HandleFunc handleShowFriends = HANDLE_FUNC(){
            const auto send_status = [&response_writer](fbs::Status status)->void{
                flatbuffers::FlatBufferBuilder builder;
                auto resp = fbs::account::CreateViewFriendResponse(builder,
                                                                   status);
                fbs::account::FinishViewFriendResponseBuffer(builder, resp);
                response_writer(builder.GetBufferPointer(), builder.GetSize());
            };
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::account::VerifyViewFriendRequestBuffer(verifier)){
                
                auto* req = fbs::account::GetViewFriendRequest(request.payload()->Data());
                const auto* session = req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    using namespace boost::property_tree;
                    
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        
                        ptree& account_tree = AccountProfiles.get_child(GetPath(username));
                        
                        flatbuffers::FlatBufferBuilder builder;
                        std::vector< flatbuffers::Offset<flatbuffers::String> > results;
                        try{
                            
                            ptree& friend_tree = account_tree.get_child(GetPath(PROFILE_FRIENDS_KEY));
                            
                            for(auto it_friend : friend_tree){
                                results.push_back(builder.CreateString(it_friend.first));
                            }
                            
                        }catch(const ptree_bad_path&){ /*No friends QQ*/ }
                        
                        auto resp = fbs::account::CreateViewFriendResponse(builder,
                                                                           fbs::Status_OK, 0,
                                                                           builder.CreateVector(results));
                        fbs::account::FinishViewFriendResponseBuffer(builder, resp);
                        
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
                Log::W("View Friend Handler") << "Payload format error" << std::endl;
                send_status(fbs::Status_PAYLOAD_FORMAT_INVALID);
            }
        };
        
        const HandleFunc handleShowFriendRequests = HANDLE_FUNC(){
            
            const auto send_status = [&response_writer](fbs::Status status)->void{
                flatbuffers::FlatBufferBuilder builder;
                auto resp = fbs::account::CreateViewPendingFriendResponse(builder,
                                                                          status);
                fbs::account::FinishViewPendingFriendResponseBuffer(builder, resp);
                response_writer(builder.GetBufferPointer(), builder.GetSize());
            };
            
            //Verify request
            flatbuffers::Verifier verifier(request.payload()->Data(), request.payload()->size());
            if(fbs::account::VerifyViewPendingFriendRequestBuffer(verifier)){
                
                auto* req = fbs::account::GetViewPendingFriendRequest(request.payload()->Data());
                const auto* session = req->session();
                
                if(session::IsSessionExist(*session)){
                    
                    using namespace boost::property_tree;
                    
                    try{
                        
                        auto username = session::GetStringValue(*session, session::SESSION_KEY_USERNAME);
                        
                        flatbuffers::FlatBufferBuilder builder;
                        std::vector< flatbuffers::Offset<flatbuffers::String> > result_vector;
                        
                        const auto& pending_users = PendingFriendRequests[username];
                        for(auto it_user : pending_users){
                            result_vector.push_back(builder.CreateString(it_user));
                        }
                        
                        auto resp = fbs::account::CreateViewPendingFriendResponse(builder,
                                                                           fbs::Status_OK, 0,
                                                                           builder.CreateVector(result_vector));
                        fbs::account::FinishViewPendingFriendResponseBuffer(builder, resp);
                        
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
                Log::W("View Pending Friend Handler") << "Payload format error" << std::endl;
                send_status(fbs::Status_PAYLOAD_FORMAT_INVALID);
            }
        };
        
    } //namespace account
    
    void InitAccountHandlers(Router& router){
        
        account::loadAccountFile();
        //Schedule write back task
        utils::AddFinalizeCallback([&]{
            //Write account data back to file
            using namespace boost::property_tree;
            try{
                write_json(ACCOUNT_FILE_NAME, AccountProfiles);
            }catch(const json_parser_error& e){
                Log::E("Account File Loader") << "Failed writing back account file: "
                                                << e.message() << std::endl;
            }
        });
        
        router.Path("/register", account::handleRegister)
        .Path("/login", account::handleLogin)
        .Path("/logout", account::handleLogout)
        .Path("/search", account::handleSearchAccount)
        .Path("/friend", account::handleShowFriends)
        .Path("/friend/pending", account::handleShowFriendRequests)
        .Path("/friend/add", account::handleAddFriends);
    }
    
}; //namespace handlers

