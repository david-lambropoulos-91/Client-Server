# Client-Server

In order to run begin by making the file
	make clean
	make

Then follow this up by running the server. The server runs with one argument and that is
the port that the server will be listening for connections on.
	./server 49660

Then in a seperate terminal run the client process. The client takes two arguments: the
address in which to connect to and the port in which to connect on.
	./client localhost 49660


