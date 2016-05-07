
#include "Account.hpp"
#include "Session.hpp"
#include <Utils.hpp>
#include <schemas/account.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

static boost::property_tree::ptree AccountProfiles;

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
                                                           session::NewSession(builder, username),
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
                                                               session::NewSession(builder, username),
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
                Log::E("Account File Loader") << "Failed writing back account file: " << e.message() << std::endl;
            }
        });
        
        router.Path("/register", account::handleRegister)
        .Path("/login", account::handleLogin)
        .Path("/logout", account::handleLogout);
    }
    
}; //namespace handlers

