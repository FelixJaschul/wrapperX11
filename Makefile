all: clean build run

clean:
	rm -rf main
	rm -rf .idea

build:
	gcc -O3 -I/opt/X11/include -Xpreprocessor -fopenmp -L/opt/X11/lib -L/opt/homebrew/Cellar/llvm/21.1.8/lib -o main main.c -lX11 -lm -lomp
	
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



