default : 
	@g++ -std=c++11 src/server.cpp -o exec/server
	@g++ -std=c++11 src/user.cpp -o exec/user
	@g++ -std=c++11 src/worker.cpp -o exec/worker -lcrypt

clean : 
	@rm -rf exec/*
	@echo " all executables cleaned up!";

