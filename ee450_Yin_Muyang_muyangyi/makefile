# Define compiler
CC=g++
CFLAGS  = -g -Wall


all: serverM serverS serverD serverU client

serverM: serverM.cpp
	g++ -o serverM serverM.cpp


serverS: serverS.cpp
	g++ -o serverS serverS.cpp


serverD: serverD.cpp
	g++ -o serverD serverD.cpp


serverU: serverU.cpp
	g++ -o serverU serverU.cpp

 
client: client.cpp
	g++ -o client client.cpp

# clean
clean:
	rm -f serverM serverS serverD serverU client