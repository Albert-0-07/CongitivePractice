cmake_minimum_required(VERSION 3.14)
project(merchant_system VERSION 1.0.0 LANGUAGES CXX)

# C++17 required for std::optional, structured bindings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -------------------------------------------------------------------
# Compiler flags
# -------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Release
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    # Debug
    set(CMAKE_CXX_FLAGS_DEBUG   "-g -O0 -DDEBUG")
elseif(MSVC)
    add_compile_options(/W4)
endif()

# -------------------------------------------------------------------
# Include directories
# -------------------------------------------------------------------
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party)

# -------------------------------------------------------------------
# Sources — Model layer
# -------------------------------------------------------------------
set(MODEL_SOURCES
    src/models/product.cpp
    src/models/store.cpp
    src/models/order.cpp
    src/models/user.cpp
    src/models/role.cpp
    src/models/log.cpp
)

# -------------------------------------------------------------------
# Sources — Config layer
# -------------------------------------------------------------------
set(CONFIG_SOURCES
    src/config/app_config.cpp
)

# -------------------------------------------------------------------
# Sources — Database layer
# -------------------------------------------------------------------
set(DATABASE_SOURCES
    src/database/dp_pool.cpp
)

# -------------------------------------------------------------------
# Sources — Service layer
# -------------------------------------------------------------------
set(SERVICE_SOURCES
    src/service/mechant_service.cpp
    src/service/admine_service.cpp
    src/service/login_servce.cpp
)

# -------------------------------------------------------------------
# Sources — Controller layer
# -------------------------------------------------------------------
set(CONTROLLER_SOURCES
    src/controller/merchant_controller.cpp
    src/controller/login_controller.cpp
    src/controller/admine_controller.cpp
)

# -------------------------------------------------------------------
# Sources — Router
# -------------------------------------------------------------------
set(ROUTER_SOURCES
    src/router/api_router.cpp
)

# -------------------------------------------------------------------
# Sources — Main
# -------------------------------------------------------------------
set(MAIN_SOURCES
    src/main.cpp
)

# -------------------------------------------------------------------
# Combine all sources
# -------------------------------------------------------------------
set(ALL_SOURCES
    ${MODEL_SOURCES}
    ${CONFIG_SOURCES}
    ${DATABASE_SOURCES}
    ${SERVICE_SOURCES}
    ${CONTROLLER_SOURCES}
    ${ROUTER_SOURCES}
    ${MAIN_SOURCES}
)

# -------------------------------------------------------------------
# Executable
# -------------------------------------------------------------------
add_executable(merchant_server ${ALL_SOURCES})

# Link pthread (if needed for mutex/threading in db pool)
find_package(Threads REQUIRED)
target_link_libraries(merchant_server Threads::Threads)

# -------------------------------------------------------------------
# Optional: find and link database client libraries
# -------------------------------------------------------------------
# find_package(MySQL REQUIRED)
# target_link_libraries(merchant_server MySQL::MySQL)
#
# or
#
# find_package(PostgreSQL REQUIRED)
# target_link_libraries(merchant_server PostgreSQL::PostgreSQL)

# -------------------------------------------------------------------
# Install
# -------------------------------------------------------------------
install(TARGETS merchant_server DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/../frontend/ DESTINATION share/merchant/frontend)

# -------------------------------------------------------------------
# Testing (placeholder)
# -------------------------------------------------------------------
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
