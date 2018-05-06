.PHONY: build
build:
	  clang $(wildcard sources/*.c externals/*/*.c) -o out `pkg-config --static --libs glfw3` -lglfw3 -g -O2
