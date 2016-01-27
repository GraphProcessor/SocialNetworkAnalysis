all: format network

format: Format.cc
	${GXX} -fpic -c Format.cc -o Format.o -std=c++11
	${GXX} -shared -o libFormat.so Format.o

network: Network.h TemporalNetwork.h TemporalNetwork.cc
	${GXX} -fpic -c LibNetwork.cc -o Network.o -std=c++11
	${GXX} -shared -o libNetwork.so Network.o ../Files/files.o