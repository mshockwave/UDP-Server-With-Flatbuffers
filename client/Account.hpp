#ifndef _CLIENT_ACCOUNT_HPP_
#define _CLIENT_ACCOUNT_HPP_

#include <iostream>

#include <Utils.hpp>

#include "Context.hpp"

namespace account {
    
    void handleRegister();
    
    void handleLogin();
    
    void handleLogout();
    
    void handleProfile();
    
} //namespace account

#endif