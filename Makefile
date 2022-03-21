all: main.cpp
	g++ -std=gnu++17 main.cpp lib/libsqlitewrap.a -lsqlite3 -o dbscan
	#mv *.o build/

#factory: lib/factory.cpp
#	g++ -std=gnu++17 lib/factory.cpp -lpthread -o factory.o

clean:
	#rm build/*.o
	rm dbscan
	rm *.db

