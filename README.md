# Overview
yabfpp (Yet Another BF++) is a cross-platform compiler of the new dialect BF++ of the prominent Brainfuck programming language.

# Supported platforms
- x86_64, Linux
- JavaScript
- Any platform you manage to build the compiler on

# BF++ features
BF++ extends BF in a number of ways
- the tape size is not limited to 30,000 cells and is virtually unlimited.
- each cell is a signed 8-bit integer.
- if/else construction. `{ifblock}{elseblock}` will run only `ifblock` if the current cell stores a non-zero value and `elseblock` will be run otherwise.
- signed integer 8-bit variables are supported. Else block is optional.
  - `^variablename` copies the value of the current cell to the variable `variablename`. 
  - `_variablename` copes the value of the variable `variable name` to the current cell. 
  - Either operation also defines the variable if it has not beed used before.
  - Variable names are strings of latin lowercase letters
- Non-negative integer literals are supported, they can follow `_`, `<`, `>`, `+` and `-`.
  - `_50` stores `50`into the current cell
  - `>50` and `<50` move the pointer right (or left) 50 times
  - `+50` and `-50` increase (decrease) the value of the current cell 50 times.
- `*` prints the value of the current cell as an integer. 
- BF used to ignore all the characters but `<`, `>`, `+`, `-`, `.`, `,`,`[`, `]`. We abandon this and require a comment to start with a semicolon. The classical behaviour is still supported via a legacy mode. 

## Code example 
Here we calculate and print 14 first Fibonacci numbers. 
```
_14               ; store the number of fib elements to calculate at the start of the tape
-2                ; which is decreased by two, as we will print the fist two elements right away
>_0*>_1*          ; initialize and print the first two elements: 0 and 1
<<                ; return to the start
[-                ; decrement the number of numbers left to calculate
>^lastbutone      ; store the last but one element into the variable lastbutone
>^last            ; store the last element into the variable last
>                 ; move to the cell we will write the next element into
_last+lastbutone  ; calculate the next element
*                 ; print it as a number
^next             ; store the next element into the next variable
<_next            ; update the last two elements
<_last
<
]
```


# Building the compiler
Dependencies are `cmake`, `clang12`, `llvm12` and `boost 1.76`. Python3 is required for testing. 

```
git clone https://github.com/akopich/yabfpp.git
cd yabfpp/
cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Release
cmake --build build --target all 
```

You may experience problems if boost was not compiled with `clang` and `c++17`enabled. The path to the custom boost can be specified by setting `BOOST_ROOT` environment variable.

To build and run the tests one can also use Python
```
cd test
python3 TestCompiler.py
```

# Building the BF++ code

Recall the code example from above. In fact, it's already [in the repo](https://github.com/akopich/yabfpp/blob/master/test/programs/fib.bfpp).
First, we build LLVM code and then get the executable with `clang`.
```
build/yabfpp test/programs/fib.bfpp -o fib.ll
clang fib.ll -o fib
./fib
```

## Building to JavaScript. 
The plan is the same, but instead of using `clang`, we will rely on `emscripten` to produce the JS code. 
```
build/yabfpp test/programs/fib.bfpp -o fib.ll --target wasm32-unknown-emscripten
emcc fib.ll -s EXIT_RUNTIME=1 -o fib.js
node fib.js
```












