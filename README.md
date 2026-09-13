# kopilang
kopilang is a very fast, high-level programming language inspired by coffee.

your code is directly translated into machine language, no one will bother you unlike an interpreted language

# syntax
use .kopi file extensions and end every block boundary with a trailing comma ",".

## 1. variable declaration
**assignment and arithmetic:**
```text
INTEGER a = 10,
INTEGER b = -5,
INTEGER sum = a + b,
INTEGER product = a * b
```

## 2. i/o (input/output)
**printing and reading input:**
prefix variables with ! to interpolate them in strings. use SAY for output and INPUT for reading values.
```text
INTEGER a = 0,
SAY "enter value.",
INPUT "!a",
SAY "Value: !a\e[0m"
```

## 3. conditional branching
**syntax structure:**
```text
INTEGER wow = 200,
IF wow == 200 <,
    SAY "wow var matches.\n",
>
```

# how to use
clone the repository and ensure **binutils** is installed:
```bash
git clone https://github.com/Skokoo/kopilang
cd kopilang
```

* **ARM64:** `clang kopilang.c -o kopi && ./kopi [file.kopi]`
* **x86_64:** `clang kopilangx86.c -o kopi && ./kopi [file.kopi]`