#include <TestCase.hpp>
#include "TestRouter.hpp"
#include "IntegrateAccountTest.hpp"
#include "IntegratePostTest.hpp"

extern "C"{
#include <unistd.h>
}

int main(){

    //Add test cases
    std::vector<TestCase*> testCases;
    testCases.push_back(new TestRouter());
    testCases.push_back(new IntegrateAccountTest());
    testCases.push_back(new IntegratePostTest());

    std::vector<TestCase*>::iterator it = testCases.begin();
    for(; it != testCases.end(); ++it){
        bool fail = false;
        usleep(500000);
        if( !((*it)->test()) ){
            Log::E("TestMain") << "Test failed on test case " << (*it)->getName() << std::endl;
            fail = true;
        }

        if((*it) != nullptr){
            TestCase* ptr = *it;
            delete ptr;
        }
        if(fail) break;
    }

    return 0;
}

