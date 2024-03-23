all: lab3 task1

lab3: lab3z.cpp
	g++ -std=c++20 lab3z.cpp -o lab3

task1: task1.cpp
	g++ task1.cpp -o task1