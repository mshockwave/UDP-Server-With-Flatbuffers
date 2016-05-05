#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <string>

namespace utils{
    
    inline void TrimString(std::string &str, char ch = ' '){
        int i;
        //Front
        for(i = 0; i < str.length(); i++){
            if(str[i] != ch) break;
        }
        if(i > 0) str.erase(0, i);
        
        //Tail
        for(i = str.length() - 1; i >= 0; i--){
            if(str[i] != ch) break;
        }
        if(i + 1 < str.length()) str.erase(i+1);
    }
    
}; //namespace utils

#endif