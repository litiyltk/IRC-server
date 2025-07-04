cmake_minimum_required(VERSION 3.16)
project(IRC-chat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим зависимости

# для сервера:
find_package(Drogon CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)

# для клиента:
find_package(cpr REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(ixwebsocket REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(wxWidgets COMPONENTS core base richtext xml html)
set(wxBUILD_SHARED ON)
include(${wxWidgets_USE_FILE})

# ----- DOMAIN ------
set(DOMAIN_INCL ${CMAKE_SOURCE_DIR}/domain/include)
set(DOMAIN_SRC ${CMAKE_SOURCE_DIR}/domain/src)

file(GLOB_RECURSE DOMAIN_SOURCES "${DOMAIN_SRC}/*.cpp" "${DOMAIN_INCL}/*.h")

add_library(general_domain_lib STATIC ${DOMAIN_SOURCES})
target_include_directories(general_domain_lib PUBLIC ${DOMAIN_INCL})
target_link_libraries(general_domain_lib PRIVATE OpenSSL::SSL OpenSSL::Crypto)

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

target_link_libraries(Server PRIVATE Drogon::Drogon general_domain_lib libpqxx::pqxx PostgreSQL::PostgreSQL)

# ------ CLIENT ------
if(wxWidgets_FOUND)
    set(CLIENT_INCL ${CMAKE_SOURCE_DIR}/client/include)
    set(CLIENT_SRC  ${CMAKE_SOURCE_DIR}/client/src)

    file(GLOB_RECURSE CLIENT_INCL_FILES ${CLIENT_INCL}/*.h)
    file(GLOB_RECURSE CLIENT_SRC_FILES  ${CLIENT_SRC}/*.cpp)


    add_executable(Client
        ${CLIENT_SRC_FILES}
        ${CLIENT_INCL_FILES}
    )


    target_include_directories(Client PRIVATE ${CLIENT_INCL})
    target_link_libraries(Client PRIVATE ${wxWidgets_LIBRARIES} cpr::cpr jsoncpp_static ixwebsocket::ixwebsocket OpenSSL::SSL OpenSSL::Crypto general_domain_lib)

    if(UNIX)
        find_package(X11 REQUIRED)
        target_link_libraries(Client PRIVATE ${X11_LIBRARIES})
    endif()
endif()

# ---- TESTS ----
enable_testing()

add_executable(DbTests
    server/tests/test_postgres.cpp
    server/src/postgres.cpp
    server/src/db_wrapper.cpp
    server/src/tagged_uuid.cpp
)

target_include_directories(DbTests PRIVATE server/include)
target_link_libraries(DbTests PRIVATE libpqxx::pqxx PostgreSQL::PostgreSQL)
find_package(GTest REQUIRED)
target_link_libraries(DbTests PRIVATE GTest::gtest GTest::gtest_main pthread)
add_test(NAME DbTestRun COMMAND DbTests)
