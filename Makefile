install: main.out
	cp main.out /usr/bin/sites

main.out: main.cpp
	g++ main.cpp -o main.out -lsqlite3

uninstall:
	rm /usr/bin/sites

clean:
	rm *.out
