cmake_minimum_required(VERSION 3.10)
project(kv_server_project)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${PROJECT_SOURCE_DIR}/server
    ${PROJECT_SOURCE_DIR}/client
    /usr/include/mysql
)

link_directories(/usr/lib /usr/lib/x86_64-linux-gnu)

# Server target
add_executable(kv_server
    server/main.cpp
    server/kv_cache.cpp
    server/db_connector.cpp
)

target_link_libraries(kv_server mysqlcppconn)

# Load generator target
add_executable(loadgen
    client/loadgen.cpp
)

# Common compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
