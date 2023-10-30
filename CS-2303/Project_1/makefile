all:shell Copy single multi
clean:
	rm *.o
shell: shell.o
	gcc shell.o -o shell
copy: copy.o
	gcc Copy.c -o Copy
single: single.o
	gcc single.c -o single
multi: multi.o
	gcc multi.c -o multi -lpthread	
