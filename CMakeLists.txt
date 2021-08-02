cmake_minimum_required(VERSION 3.21)
project(yabfpp)

SET(GCC_COVERAGE_COMPILE_FLAGS " -fexceptions")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)


link_libraries()
execute_process(COMMAND llvm-config --libs OUTPUT_VARIABLE LIBS)
execute_process(COMMAND llvm-config --system-libs OUTPUT_VARIABLE SYS_LIBS)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LDF)
#message(STATUS "Found LLVM" ${LIBS})

string(STRIP ${LIBS} LIBS)
string(STRIP ${SYS_LIBS} SYS_LIBS)
string(STRIP ${LDF} LDF)

link_libraries(${LIBS} ${SYS_LIBS} ${LDF})

execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE CMAKE_CXX_FLAGS)
string(STRIP ${CMAKE_CXX_FLAGS} CMAKE_CXX_FLAGS)

set(BOOST_ROOT /home/valerij/boost)
find_package(Boost 1.76 COMPONENTS program_options REQUIRED)

#set(Boost_INCLUDE_DIRS /home/valerij/boost)
#link_directories(/home/valerij/boost/stage/lib)

add_executable(yabfpp main.cpp Expr.cpp Expr.h ContextBuilderModule.h ContextBuilderModule.cpp BFMachine.cpp BFMachine.h parser.h parser.cpp)
target_link_libraries(yabfpp ${Boost_LIBRARIES})