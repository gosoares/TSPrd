cmake_minimum_required(VERSION 3.5)
project(TSPrd)

set(CMAKE_CXX_STANDARD 20)
set(Boost_USE_STATIC_LIBS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIL_STD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(mainFiles src/Instance.cpp src/Instance.h src/Solution.cpp src/Solution.h src/IntraSearch.cpp src/IntraSearch.h src/InterSearch.cpp src/InterSearch.h src/NeighborSearch.cpp src/NeighborSearch.h
        src/GeneticAlgorithm.cpp src/GeneticAlgorithm.h src/Split.h src/Timer.h src/Rng.h)
add_executable(TSPrd src/main.cpp ${mainFiles})

# set(CPLEX_DIR "/Applications/CPLEX_Studio201")
# set(CPLEX_ARCH "x86-64_osx")
# include_directories("${CPLEX_DIR}/cplex/include" "${CPLEX_DIR}/concert/include")
# link_directories("${CPLEX_DIR}/cplex/lib/${CPLEX_ARCH}/static_pic" "${CPLEX_DIR}/concert/lib/${CPLEX_ARCH}/static_pic")
# set(modelFiles src/MathModel.cpp src/MathModel.h)
# target_link_libraries(TSPrd ilocplex concert cplex m pthread dl)
