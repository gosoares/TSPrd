cmake_minimum_required(VERSION 3.5)
project(TSPrd)

set(CMAKE_CXX_STANDARD 20)
set(Boost_USE_STATIC_LIBS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIL_STD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wno-sign-compare")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_definitions(-DLOG_VERBOSE=false)

# fetch latest argparse
include(FetchContent)
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

add_executable(TSPrd src/main.cpp
        src/Data.hpp src/GeneticAlgorithm.h src/GeneticAlgorithm.cpp
        src/Individual.h src/Instance.h src/Instance.cpp
        src/Population.h src/Population.cpp src/Split.h src/Split.cpp
        src/NeighborSearch.hpp src/Solution.h src/Solution.cpp
        src/IntraSearches/IntraSearch.hpp src/IntraSearches/IntraSearchAlgo.hpp
        src/IntraSearches/IntraSwap.hpp src/IntraSearches/IntraReinsertion.hpp src/IntraSearches/IntraTwoOpt.hpp
        src/InterSearches/InterSearch.hpp src/InterSearches/InterSearchAlgo.hpp
        src/InterSearches/InterSwap.hpp src/InterSearches/InterRelocation.hpp src/InterSearches/DivideAndSwap.hpp
)
target_link_libraries(TSPrd argparse)

# set(CPLEX_DIR "/Applications/CPLEX_Studio201")
# set(CPLEX_ARCH "x86-64_osx")
# include_directories("${CPLEX_DIR}/cplex/include" "${CPLEX_DIR}/concert/include")
# link_directories("${CPLEX_DIR}/cplex/lib/${CPLEX_ARCH}/static_pic" "${CPLEX_DIR}/concert/lib/${CPLEX_ARCH}/static_pic")
# set(modelFiles src/MathModel.cpp src/MathModel.h)
# target_link_libraries(TSPrd ilocplex concert cplex m pthread dl)
