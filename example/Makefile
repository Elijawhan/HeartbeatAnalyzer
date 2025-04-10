


hb.o: 
	mkdir -p ./bin 
	g++ -Wno-c++11-extensions -c ../src/HeartbeatAnalyzer.cpp -o ./bin/hb.o


main : hb.o
	mkdir -p ./bin
	g++ -Wno-c++11-extensions ./main.cpp ./bin/hb.o -o ./bin/main
	./bin/main	

clean:
	rm ./bin/hb.o
	rm ./bin/main

