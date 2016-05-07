#ifndef _SERVER_INTEGRATE_SERCLI_TEST_HPP_
#define _SERVER_INTEGRATE_SERCLI_TEST_HPP_

#include "TestCase.hpp"

extern "C"{
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
}

#include <string>

#include <Utils.hpp>

#ifndef TEST_SERVER_NAME
#define TEST_SERVER_NAME    "HW2_103062222_Ser"
#endif

#ifndef TEST_SERVER_PORT
#define TEST_SERVER_PORT    6565
#endif

class IntegrateSerCliTest : public TestCase {
    
protected:
    IntegrateSerCliTest(const char* name) :
            TestCase(name){}
    
    virtual bool onServerStart() = 0;
    
    virtual bool doClient() = 0;
    
private:
    inline void killServer(pid_t pid){ kill(pid, SIGTERM); }
    
    bool doTest(){
        
        pid_t pid;
        bool result = true;
        if((pid = fork()) == 0){
            //Child. Server
            if(!onServerStart()){
                Log::E(mName) << "Error return from onServerStart for test server" << std::endl;
            }
            
            exit(0);
        }else{
            //Parent. Client
            try{
                //Wait server to start
                usleep(600000); //0.6sec
                result = doClient();
            }catch(const char* e){
                killServer(pid);
                throw e;
            }catch(const std::string &e){
                killServer(pid);
                throw e;
            }catch(...){
                Log::E(mName) << "Unknown error throw from client" << std::endl;
                result = false;
            }
            
            killServer(pid);
        }
        
        return result;
    }
};

class DefaultIntegrateSerCliTest : public IntegrateSerCliTest {
    
protected:
    DefaultIntegrateSerCliTest(const char* name) :
            IntegrateSerCliTest(name){}
    
    bool onServerStart(){
        
        auto port_str = std::to_string(TEST_SERVER_PORT);
        
        Log::V(mName) << "Starting server program "
                        << TEST_SERVER_NAME <<
                        " on port " << port_str << std::endl;
        
        execl(TEST_SERVER_NAME, TEST_SERVER_NAME, port_str.c_str(), NULL);
        
        /*
         * Only fail starting server would reach here
         */
        Log::E(mName) << "Start test server failed" << std::endl;
        return false;
    }
};

#endif