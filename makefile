all:
	g++ -std=c++17 injector_extension/dllmain.cpp -o main

clean:
	$(RM) main