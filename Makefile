all: clean build run

clean:
	rm -rf main
	rm -rf .idea

build:
	/opt/homebrew/opt/llvm/bin/clang -O3 -I/opt/X11/include -I/opt/homebrew/include -L/opt/X11/lib -L/opt/homebrew/lib -fopenmp example/main.c -lX11 -lm -lomp -o main

run:
	./main

n: clean
	nvim main.c

# GIT HELPER

MESSAGE = .

push: clean add commit
	git push

add:
	git add .

commit:
	git commit -a -m "$(MESSAGE)"



