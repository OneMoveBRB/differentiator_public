# Differentiator

## Building

### Quick start

```bash
git clone <repo-url> differentiator
cd differentiator
mkdir debug
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON -DWARNINGS_AS_ERRORS=ON
cmake --build build -j
./build/src/run examples/in.txt
```

### CMake

| Action              | Command                                                                          |
|---------------------|----------------------------------------------------------------------------------|
| Configure (Debug)   | `cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON -DBUILD_TESTS=ON`|
| Configure (Release) | `cmake -B build -DCMAKE_BUILD_TYPE=Release`                                      |
| Build               | `cmake --build build -j`                                                         |
| Run                 | `./build/src/run examples/in.txt`                                                |

### CMake options

| Option                | Default | Description                                          |
|-----------------------|---------|------------------------------------------------------|
| `CMAKE_BUILD_TYPE`    | —       | `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`   |
| `ENABLE_WARNINGS`     | `ON`    | `-Wall -Wextra -Wpedantic …`                         |
| `ENABLE_SANITIZERS`   | `OFF`   | ASan + UBSan (Debug only)                            |
| `WARNINGS_AS_ERRORS`  | `OFF`   | `-Werror`                                            |

Example with options:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DWARNINGS_AS_ERRORS=ON
cmake --build build -j
```

## Grammar

```txt
// []  - works exactly 1 time
// ()  - works 0 or 1 time
// []* - works >= 1 times
// ()* - works >= 0 times

GetExpr    := GetAddSub TOKEN_TYPE_NULL_TERMINATOR

GetAddSub  := GetMulDiv  (['+', '-'] GetMulDiv )*
GetMulDiv  := GetPow     (['*', '/'] GetPow    )*
GetPow     := GetPrimary ('^'        GetPrimary)*
GetPrimary := GetConst | GetFunc | GetVar | "(" GetExpr ")"

GetFunc  := [
        "sqrt", "ln"  , "log" ,
        "sin",  "cos" , "tan" , "cot" , 
        "sinh", "cosh", "tanh", "coth", 
        "asin", "acos", "atan", "acot"
    ] "(" GetExpr ")"

GetVar   := ['_', 'A'-'Z', 'a'-'z'] ('_', 'A'-'Z', 'a'-'z', '0'-'9')*
GetConst := ['0'-'9']* ("." ['0'-'9']*)*
```

## Example

### Original

![original](examples/original.svg)

### Differentiated

![differentiated](examples/differentiated.svg)


### Optimized

![optimized](examples/optimized.svg)

