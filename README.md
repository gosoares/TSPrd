# How to Run

## Clone the repository

```
git clone https://github.com/gosoares/TSPrd
```

## Build the binary
```
cd TSPrd
mkdir cmake-build
cmake -DCMAKE_BUILD_TYPE="Release" -Bcmake-build .
make -C cmake-build TSPrd
```

## Running an instance
```
./TSPrd Solomon/50/C101_3
```
The argument must match a file in the 'instances' folder, without the extension.

### Example output
```
RESULT 814
EXEC_TIME 3089
SOL_TIME 21
```
The times are in milliseconds.

A more detailed result is saved in the 'output' folder. Example:

```
EXEC_TIME 3089
SOL_TIME 21
OBJ 814
N_ROUTES 6
N_CLIENTS 14 3 4 10 2 17
ROUTES
43 46 49 29 23 19 16 13 11 4 2 1 3 5 
31 35 18 
20 28 24 47 
21 22 26 17 15 14 10 8 6 7 
9 12 
25 27 30 34 36 39 38 37 33 32 50 48 45 44 42 40 41
```
`EXEC_TIME` is the total execution time of the algorithm;
`SOL_TIME` is the time at which the best solution was found;
`OBJ` is the value of the solution;
`N_ROUTES` is the total number of routes in the solution;
`N_CLIENTS` is the number of clients in each route;
After `ROUTES` each line describe a route without the depot.

# Changing the algorithm parameters
The parameters are coded at the beggining of the `main.cpp` file, if you wish to change them, you can change it there and build the code again with `make -C cmake-build TSPrd `.