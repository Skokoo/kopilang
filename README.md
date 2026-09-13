# kopilang
kopiLang is a very fast high level programming language, featuring easy syntax.

the name of this programming language comes from coffee.

# how to use
please ensure your files are named using the .kopi extension (e.g., main.kopi).

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
