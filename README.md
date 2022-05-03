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

## Running a instance
```
./TSPrd Solomon/10/C101_0.5
```
The argument must match a file in the 'instances' folder, without the extension.

### Example output
```
RESULT 814
EXEC_TIME 4674
SOL_TIME 45
```
The times are in milliseconds.

A more detailed result is saved in the 'output' folder. Example:

```
EXEC_TIME 4674
SOL_TIME 45
OBJ 814
N_ROUTES 6
N_CLIENTS 13 5 6 7 3 16
ROUTES
29 31 35 33 19 16 13 10 4 1 2 3 5 
23 26 28 24 22 
20 49 50 46 47 43 
18 17 15 14 11 8 6 
12 9 7 
42 41 40 44 45 48 32 37 38 39 36 34 30 27 25 21
```

`OBJ` is the value of the solution;
`N_ROUTES` is the total number of routes in the solution; `N_CLIENTS` is the number of clients in each route.
After `ROUTES` each line describe a route without the depot.