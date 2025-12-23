run:
	g++ -std=c++17 -I/opt/X11/include -L/opt/X11/lib -o app main.cpp -lX11
	./app

clean:
	rm -rf app
	rm -rf .idea

n: clean
	nvim

# GIT HELPER

MESSAGE=.

push: clean add commit
	git push

add:
	git add .

commit:
	git commit -a -m "$(MESSAGE)"



