src = $(wildcard *.c)
obj = $(src:.c=.o)

proxy: $(obj)
	gcc -Wall -pthread -o $@ $^

.PHONY: clean
clean:
	rm -f $(obj) proxy
