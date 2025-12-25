all: clean build run

clean:
	rm -rf main
	rm -rf .idea

build:
	gcc -I/opt/X11/include -L/opt/X11/lib -o main main.c -lX11
	
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



