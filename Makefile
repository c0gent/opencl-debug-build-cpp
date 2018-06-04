buildloop: src/buildloop.cpp
	g++ -std=c++0x -pthread -o buildloop src/buildloop.cpp -lOpenCL