add_library(FunctionRenamerDeepseekAI SHARED)
set_property(TARGET FunctionRenamerDeepseekAI PROPERTY CXX_STANDARD 23)
target_sources(FunctionRenamerDeepseekAI
    PUBLIC
        FunctionRenamerDeepseekAI.cpp)

set(IDA_SDK_INC "${IDA_SDK}/include")
set(IDA_SDK_LIB "${IDA_SDK}/lib")

if (WIN32)
    if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
        set(IDA_SDK_LIB "${IDA_SDK_LIB}/x86_win_vc_32")
    else()
        set(IDA_SDK_LIB "${IDA_SDK_LIB}/x64_win_vc_64")
    endif()
elseif(APPLE)
    if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
        set(IDA_SDK_LIB "${IDA_SDK_LIB}/arm64_mac_clang_64")
    else ()
        set(IDA_SDK_LIB "${IDA_SDK_LIB}/x86_mac_clang_64")
    endif()
else ()
    if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
        set(IDA_SDK_LIB "${IDA_SDK_LIB}/x86_linux_gcc_32")
    else()
        set(IDA_SDK_LIB "${IDA_SDK_LIB}/x64_linux_gcc_64")
    endif()
endif ()

target_include_directories(FunctionRenamerDeepseekAI
    PRIVATE
        ${IDA_SDK_INC})
target_link_directories(FunctionRenamerDeepseekAI
    PRIVATE
        ${IDA_SDK_LIB})

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
        ida
        cpr::cpr
        nlohmann_json::nlohmann_json)