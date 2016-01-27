params: Parameters.h Parameters.cc
	g++ -fpic -c Parameters.cc -o Params.o -std=c++0x
	g++ -shared Params.o ../Files/files.o -o libParams.so 