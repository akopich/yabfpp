cmake_minimum_required(VERSION 3.21)
project(yabfpp)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

add_compile_options("-Wall" "-Wextra" "-pedantic")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug-specific flags
    add_compile_options("-fno-exceptions" "-g3" "-O0")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Release-specific flags
    add_compile_options("-fno-exceptions" "-O3" "-DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    # Profiling/Release with Debug Info flags
    add_compile_options("-fno-exceptions" "-fno-omit-frame-pointer" "-g" "-O2")
elseif(CMAKE_BUILD_TYPE STREQUAL "AUBSan")
    add_compile_options("-fno-exceptions" "-g" "-O1"
                        "-fsanitize=address,undefined"
                        "-fno-omit-frame-pointer"
                        "-fno-sanitize-recover=all")
    link_libraries("-fsanitize=address,undefined")
else()
    # Default flags if CMAKE_BUILD_TYPE isn't specified
    add_compile_options("-fno-exceptions" "-g" "-O0")
endif()

link_libraries()
execute_process(COMMAND llvm-config --libs OUTPUT_VARIABLE LIBS)
execute_process(COMMAND llvm-config --system-libs OUTPUT_VARIABLE SYS_LIBS)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LDF)

string(STRIP ${LIBS} LIBS)
string(STRIP ${SYS_LIBS} SYS_LIBS)
string(STRIP ${LDF} LDF)

link_libraries(${LIBS} ${SYS_LIBS} ${LDF})

execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE CMAKE_CXX_FLAGS)
string(STRIP ${CMAKE_CXX_FLAGS} CMAKE_CXX_FLAGS)

find_package(Boost 1.76 REQUIRED)
find_package(benchmark REQUIRED)

add_executable(yabfpp third_party/args.hxx main.cpp Expr.h CompilerState.h CompilerState.cpp BFMachine.cpp BFMachine.h parser.h parser.cpp CLibHandler.cpp CLibHandler.h PlatformDependent.h Source.cpp Source.h SyntaxError.h ConstantHelper.cpp ConstantHelper.h VariableHandler.h)
target_link_libraries(yabfpp ${Boost_LIBRARIES})

add_executable(SourceTest Source.cpp Source.h test/SourceTest.cpp)
target_link_libraries(SourceTest ${Boost_LIBRARIES})

add_executable(AnyTest Any.h test/AnyTest.cpp)
target_link_libraries(AnyTest ${Boost_LIBRARIES})

add_executable(ParserBench  Expr.h CompilerState.h CompilerState.cpp BFMachine.cpp BFMachine.h parser.h parser.cpp CLibHandler.cpp CLibHandler.h PlatformDependent.h Source.cpp Source.h SyntaxError.h ConstantHelper.cpp ConstantHelper.h VariableHandler.h bench/BFProgramGenerator.h bench/parserBench.cpp)
target_link_libraries(ParserBench benchmark::benchmark)

add_executable(AnyBench Any.h bench/anyBench.cpp)
target_link_libraries(AnyBench benchmark::benchmark)
