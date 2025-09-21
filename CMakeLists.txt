cmake_minimum_required(VERSION 3.21)
project(yabfpp)

SET(GCC_COVERAGE_COMPILE_FLAGS " -fexceptions")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)


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

find_package(Boost 1.76 COMPONENTS program_options REQUIRED)

add_executable(yabfpp main.cpp Expr.h CompilerState.h CompilerState.cpp BFMachine.cpp BFMachine.h parser.h parser.cpp CLibHandler.cpp CLibHandler.h PlatformDependent.h Source.cpp Source.h SyntaxErrorException.cpp SyntaxErrorException.h ConstantHelper.cpp ConstantHelper.h VariableHandler.h)
target_link_libraries(yabfpp ${Boost_LIBRARIES})

add_executable(SourceTest Source.cpp Source.h test/SourceTest.cpp)
target_link_libraries(SourceTest ${Boost_LIBRARIES})

add_executable(AnyTest Any.h test/AnyTest.cpp)
target_link_libraries(AnyTest ${Boost_LIBRARIES})
