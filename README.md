# High-Performance-ID3-Decision-Trees
## Version 1: Baseline Code
You can change the depth of the tree by manually changing depth in the functions.cpp file in the MAX_DEPTH variable. Giving a depth more than MAX_DEPTH will lead to segmentation faults.

## Version 2: Integer Encoding
We have used Integer Encoding to encode the internal table that is made from the dataset with integers.

## Version 3: Compiler Optimizations (Built over Version 2)
This version of code includes use of common compiler optimizations like loop unrolling, loop fusion, loop peeling, use of right shift operator to divide by 2 and its powers, use of pre increments instead of post increments, finding redundant operations and independent operations, use of assembly instruction for our defined use, software pipelining.

## Version 4: SIMD Optimizations (Built over Version 3)
This version of code includes use of SIMD functions to make the code much faster. SIMD kernels have been designed for high performance operations keeping in mind the Intel Broadwell architecture. 
