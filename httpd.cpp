#include <string>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include "httpd.h"
#include "Parser.h"
#include "Framer.h"
#include "Practical.h"
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <cstdlib>
#include <pthread.h>
using namespace std;

#define NUM_THREADS 5

// data structure to hold thread arguements
struct ThreadArgs{
  int clntSock = 0;
  char* doc_root = NULL;
};

void HandleTCPClient(int clntSocket, char* doc_root) {
  printf("HandleTCPClient ....\n");
  char buffer[BUFSIZE]; // Buffer for incoming request
  memset(buffer, 0, BUFSIZE);

	// create framer
	Framer* framer = new Framer();

  // Receive part of the request from the client
  ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
  if (numBytesRcvd < 0)
    DieWithSystemMessage("recv() failed");

  printf("Server received: %s", buffer);
  // Keep receiving request messages until no more
  while (numBytesRcvd > 0) { // 0 indicates end of stream
  	//printf("%s", buffer);

		// Frame by appendding new incoming Buffer
		framer->append(string((char *)buffer, numBytesRcvd));

		// process as long as there is a complete request message
		while(framer->hasRequest()){
      printf("framer has a complete request\n");
			// process received message and send once there is a complete one
			// parse the req
			Parser* parser = new Parser();
			HttpRequest req = parser->parseRequest(framer);
			// parse for response
			HttpResponse res = parser->parseResponse(req, doc_root);

			const char * out_header_buffer = res.response_msg.c_str();
			// Send back to the client
			// send header first
      printf("Sending a response %s\n", out_header_buffer);

			ssize_t numBytesSent = send(clntSocket, out_header_buffer,
				res.response_msg.size(), 0);
			if (numBytesSent < 0)
				DieWithSystemMessage("send() failed");
      printf("numBytesSent: %d", (int)numBytesSent);
			// then send file if 200
			if(res.send_file){
				int fd = open(res.file_path, S_IROTH);
        cout << "File descriptor: " << fd << endl;
				ssize_t numFileBytesSent = sendfile(clntSocket, fd, 0, res.file_size);
				if(numFileBytesSent < 0){
					DieWithSystemMessage("sendfile failed");
				}
				close(fd);
			}
      // free abs_path
      free(res.file_path);
      delete parser;

			// if client request close
			if(req.close){
				delete framer;
				close(clntSocket);
				return;
			}
		}

    // See if there is more data to receive
  	memset(buffer, 0, BUFSIZE);
  	numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);

  	if (numBytesRcvd < 0)
    		DieWithSystemMessage("recv() failed");
  }
	delete framer;
  close(clntSocket); // Close client socket
}

// Handle thread detach and argument passing
void *ThreadMain(void *threadArgs) {
  // Guarantees that thread resources are deallocated upon return
  pthread_detach(pthread_self());

  // Extract socket file descriptor from argument
  int clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
  char* doc_root = ((struct ThreadArgs *) threadArgs)->doc_root;
  free(threadArgs); // Deallocate memory for argument

  HandleTCPClient(clntSock, doc_root);

  return (NULL);
}

void start_httpd(unsigned short port, char* doc_root, char* service)
{
	cerr << "Starting server (port: " << port <<
		", doc_root: " << doc_root << ")" << endl;

	// Create socket for incoming connections
	int servSock = SetupTCPServerSocket(service);
	if(servSock < 0){
		DieWithSystemMessage("SetupTCPServerSocket() Failed");
	}


	// listen for connection
	for(;;){
		// New connection creates a connected client socket
		int clntSock = AcceptTCPConnection(servSock);

		// set out server timeout
		struct timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5;

    if (setsockopt(clntSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
           sizeof(timeout)) < 0)
    {
      DieWithUserMessage("setsockopt", "SO_RCVTIMEO");
    }

    // create thread arguments
    struct ThreadArgs *threadArgs = (struct ThreadArgs *) malloc(
        sizeof(struct ThreadArgs));
    if (threadArgs == NULL)
      DieWithSystemMessage("malloc() failed");
    threadArgs->clntSock = clntSock;
    threadArgs->doc_root = doc_root;

    // create a thread for each client connection
    pthread_t newThread;
    int thread_status = pthread_create(& newThread, NULL, ThreadMain, threadArgs);
    if (thread_status != 0)
      DieWithUserMessage("pthread_create() failed", strerror(thread_status));
    printf("with thread %ld\n", (long int) newThread);
    //HandleTCPClient(clntSock, doc_root); // Process client
    //close(clntSock);
	}
	// NOT REACHED
	close(servSock);
}
