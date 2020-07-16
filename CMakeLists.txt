cmake_minimum_required(VERSION 3.16)
project(TSPrd)

set(CMAKE_CXX_STANDARD 14)
set(Boost_USE_STATIC_LIBS ON )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DIL_STD" )

include_directories("/opt/ibm/ILOG/CPLEX_Studio129/cplex/include" "/opt/ibm/ILOG/CPLEX_Studio129/concert/include")
link_directories("/opt/ibm/ILOG/CPLEX_Studio129/cplex/lib/x86-64_linux/static_pic" "/opt/ibm/ILOG/CPLEX_Studio129/concert/lib/x86-64_linux/static_pic")

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(TSPrd main.cpp Instance.cpp Instance.h Solution.cpp Solution.h SplitMathModel.cpp SplitMathModel.h NeighborSearch.cpp NeighborSearch.h Functions.cpp Functions.h)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

target_link_libraries(TSPrd ilocplex concert cplex m pthread dl)