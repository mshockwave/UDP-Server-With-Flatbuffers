#include <Utils.hpp>

//Hide symbol
static std::vector<utils::FinalizeCallback> FinalizeCallbacks;

namespace utils {
    
    void AddFinalizeCallback(const FinalizeCallback& callback){
        FinalizeCallbacks.insert(FinalizeCallbacks.begin(), callback);
    }
    void PushBackFinalizeCallback(const FinalizeCallback& callback){
        FinalizeCallbacks.push_back(callback);
    }
    void InsertFinalizeCallback(unsigned int index, const FinalizeCallback& callback){
        if(index < FinalizeCallbacks.size()){
            FinalizeCallbacks.insert(FinalizeCallbacks.begin() + index, callback);
        }else{
            PushBackFinalizeCallback(callback);
        }
    }
    
    void DoFinalize(){
        std::vector<FinalizeCallback>::iterator it_callback;
        for(it_callback = FinalizeCallbacks.begin();
            it_callback != FinalizeCallbacks.end(); ++it_callback){
            auto callback = *it_callback;
            callback();
        }
    }
    
}; //namespace handlers