cmake_minimum_required(VERSION 3.5)
project(TSPrd)

set(CMAKE_CXX_STANDARD 20)
set(Boost_USE_STATIC_LIBS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIL_STD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(CPLEX_DIR "/Applications/CPLEX_Studio201")
set(CPLEX_ARCH "x86-64_osx")
include_directories("${CPLEX_DIR}/cplex/include" "${CPLEX_DIR}/concert/include")
link_directories("${CPLEX_DIR}/cplex/lib/${CPLEX_ARCH}/static_pic" "${CPLEX_DIR}/concert/lib/${CPLEX_ARCH}/static_pic")

set(mainFiles Instance.cpp Instance.h Solution.cpp Solution.h NeighborSearch.cpp NeighborSearch.h
        GeneticAlgorithm.cpp GeneticAlgorithm.h Split.h Timer.h)
set(modelFiles MathModel.cpp MathModel.h)
add_executable(TSPrd main.cpp ${mainFiles})
add_executable(Runner Runner.cpp)
add_dependencies(Runner TSPrd)
add_executable(Param ParameterTuning.cpp ${mainFiles})

# target_link_libraries(TSPrd ilocplex concert cplex m pthread dl)
