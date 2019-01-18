# Binary Bomb Diffusion
### Description:
This project helps to practice a variety of terminal tools, such as objdump and strings, especially the debugging environment GDB, for reverse engineering. The goal of this project is to diffuse a multi-phase 32-bit binary bomb. A binary bomb is a program that consists of a sequence of phases. Each phase expects a particular string as inputs which can either be provided through the terminal (stdin) or through a file.

Part of Provied Source Code:
```C
int main (int argc , char argv []) {
    FILE fp;
    if (argc > 1) {
        fp = fopen(argv[1], r);
    }
    
    if (fp == NULL) {
        goto usage ret;
    }
    else {
        fp = stdin;
    }
    initialize();
    phase0(fp);
    phase1(fp);
    phase2(fp);
    phase3(fp);
    phase4(fp);
    phase5(fp);
    phase6(fp);
    
    return 0 ;
    
    usage ret :
    printf(Usage: %s OR%s <file>\n, argv[0], argv[0]);
return 1 ;
}
```

### Highlights:
1. Use **b \*address** to break at an address and examine the values of registers and memory.
2. **objdump -t**. This command will print out the bombs symbol table.
3. **objdump -d -M intel**. Use this to disassemble all of the code in the bomb.
4. **strings**. This utility will display the printable strings in your bomb.

### References:
1. [Binary Bomb Lab](http://zpalexander.com/binary-bomb-lab-phase-1/)