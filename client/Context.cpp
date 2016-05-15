#include "Context.hpp"

#include <map>

static std::map<context::Screen, context::ScreenHandler> ScreenMap;

namespace context {
    
    std::string CurrentTokenStr("");
    
    int SocketFd = -1;
    
    const char PROMPT_CHAR = '>';
    
    std::string Username("");
    
    namespace post{
        long MaxPid = -1;
        long CurrentPid = -1;
        
        long MaxCid = -1;
        long CurrentCid = -1;
    } //namespace post
    
    namespace account{
        std::vector<std::string> PendingFriends;
    } //namespace account
    
    namespace msg{
        
        channel_id_t CurrentChannelId;
        std::vector<channel_id_t> Channels;
        std::set<size_t> GroupChannelIndex;
        
        std::unordered_map<std::string, int> TransferFdMap;
        
    } //namespace msg
    
    ScreenHandler& GetScreen(Screen scr){
        auto it_screen = ScreenMap.find(scr);
        return (it_screen == ScreenMap.end())? ScreenMap[Screen::ENTRY] : (it_screen->second);
    }
    void AddScreen(Screen scr, const ScreenHandler& handler){
        ScreenMap[scr] = handler;
    }
    
    void InitScreens(){
        AddScreen(Screen::ENTRY, SCREEN_HANDLER(){
            
            PrintDivideLine();
            std::cout << "Welcome!!" << std::endl;
            PrintDivideLine();
            
            std::cout << "[R]egister\t" << "[L]ogin" << std::endl << "[Q]uit" << std::endl;
            std::cout << PROMPT_CHAR;
            
            char input_cmd;
            std::cin >> input_cmd;
            
            switch(std::tolower(input_cmd)){
                case 'l':{
                    return Screen::LOGIN;
                }
                    
                case 'r':{
                    return Screen::REGISTER;
                }
                    
                case 'q':{
                    return Screen::QUIT;
                }
                    
                default:
                    std::cout << "Unrecognized input: " << input_cmd << std::endl;
                    return Screen::STAY;
            }
        });
        
        AddScreen(Screen::MAIN, SCREEN_HANDLER(){
            PrintDivideLine();
            
            std::cout << "[A]dd Post\t" << "[V]iew Posts" << std::endl;
            std::cout << "[S]earch Accounts\t" << "[F]riend" << std::endl;
            std::cout << "My [P]rofile" << std::endl;
            std::cout << "[M]essage" << std::endl;
            std::cout << "[L]ogout" << std::endl;
            std::cout << PROMPT_CHAR;
            
            char input_cmd;
            std::cin >> input_cmd;
            
            switch(std::tolower(input_cmd)){
                case 'a':{
                    return Screen::ADD_POST;
                }
                    
                case 'v':{
                    return Screen::VIEW_NEXT_POST;
                }
                
                case 'l':{
                    return Screen::LOGOUT;
                }
                    
                case 's':{
                    return Screen::SEARCH_ACCOUNT;
                }
                    
                case 'f':{
                    return Screen::FRIEND_ENTRY;
                }
                    
                case 'p':{
                    return Screen::GET_PROFILE;
                }
                    
                case 'm': {
                    return Screen::MSG_ENTRY;
                }
                    
                default:{
                    std::cout << "Unrecognized input: " << input_cmd << std::endl;
                    return Screen::STAY;
                }
            }
        });
    }
}