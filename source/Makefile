all: example.exe


example.exe: fdt.c test.c demo.c
	gcc -o $@ $^ -Dx86_64
	@strip $@


.PHONY: clean
clean:
	rm -f example.exe
