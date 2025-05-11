all: test-dt.c test.exe run

run: test.exe
	@printf "run test.exe >>>\n"
	./test.exe

test.exe: fdt.c test-ut.c test-dt.c
	@printf "build test.exe >>>\n"
	gcc -o $@ $^ -Dx86_64 -Wunused-function -Wall -Wextra -Werror 
	@strip $@

test-dt.c: test-dt.dts
	@printf "build device tree >>>\n"
	./fdtc.exe -c $@ $^

.PHONY: clean
clean:
	rm -f test-dt.c test.exe
