# Linux Shell Implementaion
### Description:
This project helps to practice a variety of system calls, such as fork(), exec(), dup(), signal(). This is a basic linux shell that supports list commands, pipe commands, I/O redirection, and asynchronous commands; it also changed the default behavior when users hit Ctrl+C.

### Highlights:
1. Use different structs to store different parts of a command.
2. Effectively parse the command using a parse-tree data structure.
3. Recursively call to run different parts of the command.

### Build:
```sh
make shell
```

### References:
1. sh.c from XV6.