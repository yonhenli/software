all: shell

run: all
	./shell

shell: shell.c
	gcc -static shell.c -o $@

clean:
	rm -f shell