cmake_minimum_required(VERSION 2.8)
project(NP_HW2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(INCLUDE_DIR 
    include
    flatbuffers/include)

include_directories(${INCLUDE_DIR})

set(FLATBUFFER_SCHEMA_HEADERS
        flatbuffers/include/schemas/account.h
        flatbuffers/include/schemas/post.h
        flatbuffers/include/schemas/types_generated.h
        flatbuffers/include/schemas/packet_generated.h)

set(SHARED_SOURCE_FILES
        include/Log.hpp Log.cpp
        include/Utils.hpp Utils.cpp)

set(SERVER_SOURCE_FILES
        ${SHARED_SOURCE_FILES}
        ${FLATBUFFER_SCHEMA_HEADERS}
        server/Router.hpp server/Router.cpp
        server/Types.hpp server/Types.cpp
        server/Account.hpp server/Account.cpp
        server/Session.hpp server/Session.cpp)
add_executable(HW2_103062222_Ser ${SERVER_SOURCE_FILES} server/Main.cpp)

set(SERVER_TEST_HEADER_FILES
        include/IntegrateSerCliTest.hpp
        include/TestCase.hpp
        server/tests/TestRouter.hpp
        server/tests/IntegrateAccountTest.hpp)
add_executable(ServerTests server/tests/TestMain.cpp
                          ${SERVER_SOURCE_FILES}
                          ${SERVER_TEST_HEADER_FILES})
add_dependencies(ServerTests HW2_103062222_Ser)

set(CLIENT_SOURCE_FILES
        ${SHARED_SOURCE_FILES}
        ${FLATBUFFER_SCHEMA_HEADERS}
        client/Context.hpp client/Context.cpp
        client/Account.hpp client/Account.cpp
        client/Post.hpp client/Post.cpp)
add_executable(HW2_103062222_Cli ${CLIENT_SOURCE_FILES} client/Main.cpp)

set(CLIENT_TEST_HEADER_FILES
        include/TestCase.hpp)
add_executable(ClientTests client/tests/TestMain.cpp
                          ${CLIENT_SOURCE_FILES}
                          ${CLIENT_TEST_HEADER_FILES})

