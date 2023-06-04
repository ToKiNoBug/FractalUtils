find_package(fmt QUIET)

if(${fmt_FOUND})
    return()
endif()

include(FetchContent)

message(Downloading fmtlib...)
FetchContent_Declare(fmtlib
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.0.0
    OVERRIDE_FIND_PACKAGE)

FetchContent_MakeAvailable(fmtlib)

find_package(fmt REQUIRED)
