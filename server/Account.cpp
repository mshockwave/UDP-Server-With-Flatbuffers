
#include "Account.hpp"
#include <Utils.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

static boost::property_tree::ptree AccountProfiles;

namespace handlers{
    
    void loadAccountFile(){
        using namespace boost::property_tree;
        try{
            read_json(ACCOUNT_FILE_NAME, AccountProfiles);
        }catch(const json_parser_error& e){
            Log::E("Account File Loader") << "Failed loading account file: " << e.message() << std::endl;
        }
    }
    
    void InitAccountHandlers(Router* router){
        
        loadAccountFile();
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
        
        
    }
    
}; //namespace handlers

