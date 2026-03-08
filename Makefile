build: main.c
	gcc -o md2pdf main.c -Wall -Wextra -ggdb

.PHONY:
clean:
	rm *.pdf
	rm md2pdf
