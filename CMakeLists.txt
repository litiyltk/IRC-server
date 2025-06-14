cmake_minimum_required(VERSION 3.16)
project(IRC-chat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим зависимости
# для сервера:
find_package(Drogon REQUIRED) 
find_package(Boost REQUIRED COMPONENTS log log_setup) # для сервера TODO добавить ЛОГГИРОВАНИЕ
# для клиента:
find_package(cpr REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(ixwebsocket REQUIRED)
find_package(OpenSSL REQUIRED)

# ----- SERVER ------
set(SERVER_INCL ${CMAKE_SOURCE_DIR}/server/include)
set(SERVER_SRC  ${CMAKE_SOURCE_DIR}/server/src)

file(GLOB_RECURSE SERVER_INCL_FILES ${SERVER_INCL}/*.h)
file(GLOB_RECURSE SERVER_SRC_FILES  ${SERVER_SRC}/*.cpp)

add_executable(Server
    ${SERVER_SRC_FILES}
    ${SERVER_INCL_FILES}
)

target_include_directories(Server PRIVATE ${SERVER_INCL})
target_link_libraries(Server PRIVATE Drogon::Drogon ${Boost_LIBRARIES})

# ------ CLIENT ------
set(CLIENT_INCL ${CMAKE_SOURCE_DIR}/client/include)
set(CLIENT_SRC  ${CMAKE_SOURCE_DIR}/client/src)

file(GLOB_RECURSE CLIENT_INCL_FILES ${CLIENT_INCL}/*.h)
file(GLOB_RECURSE CLIENT_SRC_FILES  ${CLIENT_SRC}/*.cpp)

add_executable(Client
    ${CLIENT_SRC_FILES}
    ${CLIENT_INCL_FILES}
)

target_include_directories(Client PRIVATE ${CLIENT_INCL})
target_link_libraries(Client PRIVATE cpr::cpr jsoncpp_static ixwebsocket::ixwebsocket OpenSSL::SSL OpenSSL::Crypto)
