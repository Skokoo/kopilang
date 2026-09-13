# kopilang
kopiLang is a very fast high level programming language.

the name of this programming language comes from coffee.

# syntax
please ensure your source files are named using the .kopi extension (e.g., main.kopi). Every block boundaries must end with a trailing comma ,.

## 1. variable declaration
**variable assignment:**

```text
INTEGER a = 10,
INTEGER b = -5
```

**mathematical arithmetic:**

supported operators: +, -, *, /.
```text
INTEGER sum = a + b,
INTEGER product = a * b
```

## 2. I/O
printing text and var's (SAY):

to print variables inside a string, prefix the variable name with an exclamation mark "!". You can use "\n" for a newline and "\e" for ANSI color escape codes.
```text
SAY "Hello guys.\n",
SAY "The value of variable sum is: \e[1;32m!sum\e[0m"
```

**reading user input (INPUT):**

to capture integer input directly from the keyboard register, use the "INPUT" token targeted at a predefined variable prefixed with "!".
```text
INTEGER a = 0,
SAY "enter instruction seed: ",
INPUT "!a"
```

## 3. conditional branching
**syntax Structure:**

supported comparison operators: ==, !=, >, <, >=, <=.
```text
INTEGER wow = 200,
IF wow == 200 <,
    SAY "wow var matches.\n",
>
```

# how to use
to get started with this language, execute the commands below:

```bash
git clone https://github.com/Skokoo/kopilang
cd kopilang
```

before using this programming language, please ensure you have the binutils package installed. otherwise, the program will encounter compilation errors.

**For arm64:**
```bash
clang kopilang.c -o kopi
./kopi [Your .kopi file]
```

**For x86_64:**
```bash
clang kopilangx86.c -o kopi
./kopi [Your .kopi file]
```
