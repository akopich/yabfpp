# Overview
yabfpp (Yet Another BF++) is a cross-platform compiler of the new dialect BF++ of the prominent Brainfuck programming language.

# Supported platforms
- x86_64, Linux
- JavaScript
- Any platform you manage to build the compiler on

# BF++ features
BF++ extends BF in a number of ways
- functions. See below. 
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
... but I promised you functions, right? 

## Functions
In BF++ every function takes an arbitrary (yet, known in compile-time, variadic functions are not supported) number of signed 8-bit integers and returns a single signed 8-bit integer. On every call of a function a new tape is created for the function to operate on it. Each function has its own variable scope. To return the value stored in the current cell from the function, use `\`. Multiple returns are supported. If control flow reaches the end of the function, the value stored in the current cell is returned. The returned value is written into caller's tape, into caller's current cell. 

A function can be declared as 
```
@functionname(argone, argtwo, argthree) {[functionbody]}
```
and called as 
```
$functionname(argone, argtwo, argthree)
```
The provided arguments can be either variables or integer literals. 

### Recursive calculation of Fibonacci numbers.

```
@fib(n) {_n{-                   ; write n into the current cell and check if it's zero. If not, decrement it
              {^prev            ; check if it's 1 (if the current cell is 0) . If not, store n-1 in prev variable
                $fib(prev)      ; call the function fib recursively for n-1
                ^fibprev        ; store the result
                _prev           ; get n-1 into the current cell
                -               ; decrement it
                ^prevprev       ; store n-2 into prevprev
                $fib(prevprev)  ; another recursive call for, now for n-2
                +fibprev}       ; add fib(n-1) to the result
             {_1}}              ; return 1 if n == 1
          {_0}                  ; return 0 if n == 0
         }                      ; no need to return explicitly, but you are free to add \.

_12^x$fib(x)*                   ; store 12 into a variable named x and call fib. Print the result afterwards.
$fib(12)*                       ; or you can just call the function with an integer literal. 
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












