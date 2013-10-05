#include "proxy.h"

int main(int argc, char **argv){
	if(argc == 1 && (strcmp(argv[1], "-h"))){
			return printHelp(); 
	}	
			
	switch(argc){
		case 2: return Client(argv); 
		case 3: return Server(argv);
		default: 
				printf("Invalid number of arguments");
				return 1; 
	}

}

int printHelp(){
	printf("\nFor USE:\n"); 
	printf("If setting up for server use:\n\t./proxy <port> <local interface>\n"); 
	printf("If setting up for client use:\n\t./proxy <remote host> <remote port> <local interface>\n"); 
	printf("\nThank you for using our proxy!\n"); 
}

int Client(char **argv){

}

int Server(char **argv){
	sockaddr_in sockAddr; 
	int serverSocket; 
	boolean binder, listener; 
	int port; 

	scanf(argv[1], "%d", port); //converts the port from ascii to int
	
	/*create the socket*/
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if(serverSocket == -1){ 
		printf("Failed to create the socket");
		return 1; 
	}

	/*set the port for the socket*/
	memset(&sockAddr, 0, sizeof(sockAddr)); 
	sockAddr.sin_family = AF_INET; 
	sockAddr.sin_addr.s_addr = INADDR_ANY;
	sockAddr.sin_port = htos(port);
	
	/*bind the socket to the port*/
	binder = bind((serverSocket, (sockaddr)&sockAddr, sizeof(sockAddr)));
	if(binder){
		printf("Failed to bind to port\n"); 
		close(serverSocket);
		return 1; 
	}

	/*set the amount of connections in the queue for the socket*/
	listener = listen(serverSocket, 10); 
	if(listener == -1){ 
		printf("Failed to listen to port");
		close(serverSocket); 
		return 1; 
	}

}
