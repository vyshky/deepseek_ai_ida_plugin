link_directories(${IDA_SDK}/lib)
include_directories(${IDA_SDK}/include)

add_library(FunctionRenamerDeepseekAI SHARED)
target_sources(FunctionRenamerDeepseekAI
    PUBLIC
        FunctionRenamerDeepseekAI.cpp)

include(FetchContent)

FetchContent_Declare(cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr
    GIT_VERSION 1.12.0)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_VERSION "JSON for Modern C++ version 3.12.0")

FetchContent_MakeAvailable(cpr json)

target_link_libraries(FunctionRenamerDeepseekAI
    PRIVATE
        cpr::cpr
        nlohmann_json::nlohmann_json)