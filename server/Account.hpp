#ifndef _ACCOUNT_HPP_
#define _ACCOUNT_HPP_

#include "Router.hpp"
#include "Types.hpp"

namespace handlers{
    
#define PROFILE_PASSWORD_KEY    "password"
#define PROFILE_NIKNAME_KEY     "nickname"
    
#define PROFILE_FRIENDS_KEY      "friends"
    /*
     friends: {
        "username1": "placeholder",
        "username2": "placeholder",
        ...
     }
     */
    
#define PROFILE_BIRTHDAY_KEY    "birthday"
    
    void InitAccountHandlers(Router&);
    
}; //namespace handlers

#endif