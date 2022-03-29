all:
	g++ -std=c++17 dllmain.cpp -o main

clean:
	$(RM) main