#ifndef _SERVER_TESTS_ROUTER_HPP_
#define _SERVER_TESTS_ROUTER_HPP_

#include <TestCase.hpp>

#include "../Router.hpp"

class TestRouter : public TestCase {
    
public:
    TestRouter() :
        TestCase("TestRouter"){}
    
private:
    
    bool testNormalPathCallbacks(){
        Log::V(mName) << "Testing Router: Normal path callbacks" << std::endl;
        
        Router router;
        router.Path("/foo", HANDLE_FUNC(){
            Log::D("") << "Foo Handler. Path: " << RequestPathStr(request) << std::endl;
        })
        .Path("/bar", HANDLE_FUNC(){
            Log::D("") << "Bar Handler. Path: " << RequestPathStr(request) << std::endl;
        })
        .Path("/foo/sub-foo", HANDLE_FUNC(){
            Log::D("") << "Sub-Foo Handler. Path: " << RequestPathStr(request) << std::endl;
        });
        
        auto empty_response_writer = [](const byte_t* content, size_t size)->ssize_t {
            return 0;
        };
        
        {
            //Path: '/foo'
            flatbuffers::FlatBufferBuilder builder;
            auto result = fbs::CreateRequestPacket(builder, builder.CreateString("/foo"));
            fbs::FinishRequestPacketBuffer(builder, result);
            Request req(*fbs::GetRequestPacket(builder.GetBufferPointer()));
            
            router.Process(req, empty_response_writer);
        }
        
        {
            //Path: '/bar'
            flatbuffers::FlatBufferBuilder builder;
            auto result = fbs::CreateRequestPacket(builder, builder.CreateString("/bar"));
            fbs::FinishRequestPacketBuffer(builder, result);
            Request req(*fbs::GetRequestPacket(builder.GetBufferPointer()));
            
            router.Process(req, empty_response_writer);
        }
        
        {
            //Path: '/foo/sub-foo'
            flatbuffers::FlatBufferBuilder builder;
            auto result = fbs::CreateRequestPacket(builder, builder.CreateString("/foo/sub-foo"));
            fbs::FinishRequestPacketBuffer(builder, result);
            Request req(*fbs::GetRequestPacket(builder.GetBufferPointer()));
            
            router.Process(req, empty_response_writer);
        }
        
        return true;
    }
    
    bool testSubRouters(){
        Log::V(mName) << "Testing Router: With sub routers" << std::endl;
        
        Router main_router;
        main_router.Path("/foo", HANDLE_FUNC(){
            Log::D("") << "Foo Handler. Path: " << RequestPathStr(request) << std::endl;
        })
        .Path("/foo/sub-foo", HANDLE_FUNC(){
            Log::D("") << "Sub-Foo Handler. Path: " << RequestPathStr(request) << std::endl;
        });
        
        Router sub_router;
        sub_router.Path("/dummy1", HANDLE_FUNC(){
            Log::D("") << "Sub-Router dummy1 Handler. Path: " << RequestPathStr(request) << std::endl;
        })
        .Path("/dummy1/dummy2", HANDLE_FUNC(){
            Log::D("") << "Sub-Router dummy1/dummy2 Handler. Path: " << RequestPathStr(request) << std::endl;
        })
        .Path("/dummy3", HANDLE_FUNC(){
            Log::D("") << "Sub-Router dummy3 Handler. Path: " << RequestPathStr(request) << std::endl;
        });
        
        main_router.Path("/foo/sub-namespace", sub_router);
        
        auto empty_response_writer = [](const byte_t* content, size_t size)->ssize_t {
            return 0;
        };
        
        {
            //Path: '/foo/sub-foo'
            flatbuffers::FlatBufferBuilder builder;
            auto result = fbs::CreateRequestPacket(builder, builder.CreateString("/foo/sub-foo"));
            fbs::FinishRequestPacketBuffer(builder, result);
            Request req(*fbs::GetRequestPacket(builder.GetBufferPointer()));
            
            main_router.Process(req, empty_response_writer);
        }
        
        {
            //Path: '/foo/sub-namespace/dummy1'
            flatbuffers::FlatBufferBuilder builder;
            auto result = fbs::CreateRequestPacket(builder, builder.CreateString("/foo/sub-namespace/dummy1"));
            fbs::FinishRequestPacketBuffer(builder, result);
            Request req(*fbs::GetRequestPacket(builder.GetBufferPointer()));
            
            main_router.Process(req, empty_response_writer);
        }
        
        {
            //Path: '/foo/sub-namespace/dummy1/dummy2'
            flatbuffers::FlatBufferBuilder builder;
            auto result = fbs::CreateRequestPacket(builder, builder.CreateString("/foo/sub-namespace/dummy1/dummy2"));
            fbs::FinishRequestPacketBuffer(builder, result);
            Request req(*fbs::GetRequestPacket(builder.GetBufferPointer()));
            
            main_router.Process(req, empty_response_writer);
        }
        
        {
            //Path: '/foo/sub-namespace/dummy3'
            flatbuffers::FlatBufferBuilder builder;
            auto result = fbs::CreateRequestPacket(builder, builder.CreateString("/foo/sub-namespace/dummy3"));
            fbs::FinishRequestPacketBuffer(builder, result);
            Request req(*fbs::GetRequestPacket(builder.GetBufferPointer()));
            
            main_router.Process(req, empty_response_writer);
        }
        
        return true;
    }
    
    bool doTest(){
        
        bool result = true;
        result &= testNormalPathCallbacks();
        result &= testSubRouters();
        
        return result;
    }
};

#endif