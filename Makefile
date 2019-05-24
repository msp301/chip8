BINARY=chip8

$(BINARY):
	gcc -o $(BINARY) chip8.c

clean:
	rm -f chip8
