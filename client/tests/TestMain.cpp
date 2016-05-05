#include <TestCase.hpp>

int main(){

    //Add test cases
    std::vector<TestCase*> testCases;
    //TODO: Add test cases

    std::vector<TestCase*>::iterator it = testCases.begin();
    for(; it != testCases.end(); ++it){
        bool fail = false;
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

