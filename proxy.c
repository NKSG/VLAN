#include "proxy.h"

int main(int argc, char **argv){
	if(argc == 1 || (strcmp(argv[1], "-h") == 0)){
			return printHelp(); 
	}	
			
	switch(argc){
		case 3: return Server(argv); 
		case 4: return Client(argv);
		default: 
				printf("Invalid number of arguments");
				return 1; 
	}

}

int printHelp(){
	printf("\nFor USE:\n"); 
	printf("If setting up for server use:\n\t./proxy <port> <local interface>\n\n"); 
	printf("If setting up for client use:\n\t./proxy <remote host> <remote port> <local interface>\n"); 
	printf("\nThank you for using our proxy!\n"); 
}

/**************************************************
* allocate_tunnel:
* open a tun or tap device and returns the file
* descriptor to read/write back to the caller
* from project pdf
*****************************************/
int allocate_tunnel(char *dev, int flags) {
    int fd, error;
    struct ifreq ifr;
    char *device_name = "/dev/net/tun";
    if( (fd = open(device_name , O_RDWR)) < 0 ) {
      perror("error opening /dev/net/tun");
      return fd;
    }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = flags;
    if (*dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }
    if( (error = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
      perror("ioctl on tap failed");
      close(fd);
      return error;
    }
    strcpy(dev, ifr.ifr_name);
    return fd;
}

/**************************************************************************
 * cread: read routine that checks for errors and exits if an error is    *
  *        returned.                                                       *
   **************************************************************************/
int cread(int fd, char *buf, int n){

    int nread;

    if((nread=read(fd, buf, n)) < 0){
        perror("Reading data");
        exit(1);
    }
    return nread;
}

/**************************************************************************
* cwrite: write routine that checks for errors and exits if an error is  *
*         returned.                                                      *
**************************************************************************/
int cwrite(int fd, char *buf, int n){

    int nwrite;

    if((nwrite=write(fd, buf, n)) < 0){
        perror("Writing data");
        exit(1);
    }
    return nwrite;
}

/**************************************************************************
* read_n: ensures we read exactly n bytes, and puts them into "buf".     *
*         (unless EOF, of course)                                        *
**************************************************************************/
int read_n(int fd, char *buf, int n) {

    int nread, left = n;

    while(left > 0) {
        if ((nread = cread(fd, buf, left)) == 0){
            return 0 ;
        }else {
            left -= nread;
            buf += nread;
        }
    }
    return n;
}

/*
 * reads from the tap device and writes to the port
 */
int Client(char **argv){
	sockaddr_in servSockAddr; 
	sockaddr_in myaddr; 
	int clientSocket, connection; 
	char buffer[BUFSIZE]; 
	boolean binder, listener; 
	int host, port, reader; 
    char remote_ip[16] = "";
    //tun variables
    int maxfd;
    int tap_fd;
    int sock_fd, net_fd, optval = 1;
    unsigned long int tap2net = 0, net2tap = 0;
    uint16_t nread, nwrite, plength;
    int16_t typeMes;
    char *if_name = (char *)calloc(100, sizeof(char)); 
	struct hostent *hp; 

	
	strcpy(remote_ip, argv[1]);	//gets the remote host
	port = atoi(argv[2]);	//gets the port 
	strcpy(if_name, argv[3]); //gets the name of the tap device	

	/*create the socket*/
	clientSocket = socket(AF_INET, SOCK_STREAM, 0); 
	if(clientSocket < 0){
		printf("Failed to create socket\n"); 
		return 1; 
	}

    /* bind to an arbitrary return address */
    /* because this is the client side, we don't care about the */
    /* address since no application will connect here  --- */
    /* INADDR_ANY is the IP address and 0 is the socket */
    /* htonl converts a long integer (e.g. address) to a network */
    /* representation (agreed-upon byte ordering */

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);
/*
    if (bind(clientSocket, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        close(clientSocket);
        return -1;
    }
	*/



	/*set the (remote) port*/
	memset(&servSockAddr, 0, sizeof(servSockAddr));
	servSockAddr.sin_family = AF_INET; 
	servSockAddr.sin_addr.s_addr = inet_addr(remote_ip);
	if(servSockAddr.sin_addr.s_addr == -1){
		hp = gethostbyname(remote_ip); 
		if(hp == NULL){
			fprintf(stderr, "%s is unreachable or doesn't exist\n", remote_ip); 
			return 0;
		}
		bcopy(hp->h_addr, &servSockAddr.sin_addr, hp->h_length); 
	}
	servSockAddr.sin_port = htons(port);
	
	/*set the (remote) host*/
    /*
	if(inet_pton(AF_INET, argv[1], &servSockAddr.sin_addr)<0){
		printf("Failed to convert IPv4 addr to binary\n");
		return 1; 
	}
    */

	connection = connect(clientSocket, (sockaddr *) &servSockAddr, sizeof(servSockAddr));

	if(connection < 0){
		printf("Failed to connect to host\n"); 
		close(clientSocket); 
        exit(1);
	}

    net_fd = clientSocket;

    /* Connecting to tap. now you can read/write on tap_fd (used combo of tunnel function 
        from assignment pdf  simpletun c file */ 

    //from socket to net
    if ( (tap_fd = allocate_tunnel(if_name, IFF_TAP | IFF_NO_PI)) < 0 ) {
        perror("Opening tap interface failed! \n");
        exit(1);
    }

    /* use select() to handle two descriptors at once */
    maxfd = (tap_fd > net_fd)?tap_fd:net_fd;

    while(1){

		printf("asdf. tap_fd:%i\n",tap_fd);

        /* have to figure out how to read from the tap
        * device and write out to the socket. 
        */
        int ret;
        fd_set rd_set;

        FD_ZERO(&rd_set);
        FD_SET(tap_fd, &rd_set); FD_SET(net_fd, &rd_set);

        ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);

        if (ret < 0 && errno == EINTR){
            continue;
        }

        if (ret < 0) {
            perror("select()");
            exit(1);
        }

	printf("Past ret < 0 second,tap_fd: %i\n",tap_fd);

        if(FD_ISSET(tap_fd, &rd_set)) {
            /* data from tun/tap: just read it and write it to the network */
			printf("reading from tap, writing to the network\n");
            nread = cread(tap_fd, buffer, BUFSIZE);

            tap2net++;
            printf("TAP2NET %lu: Read %d bytes from the tap interface\n", tap2net, nread);

            /* write length + packet */
            plength = htons(nread);
            nwrite = cwrite(net_fd, (char *)&plength, sizeof(plength));
            nwrite = cwrite(net_fd, buffer, nread);

            printf("TAP2NET %lu: Written %d bytes to the network\n", tap2net, nwrite);
        }



        if(FD_ISSET(net_fd, &rd_set)) {
            /* data from the network: read it, and write it to the tun/tap interface. 
            * we need to read the length first, and then the packet */
			printf("Client::reading from the network, writing to the tap\n");
            /* read length */
            nread = read_n(net_fd, (char *)&plength, sizeof(plength));
            if(nread == 0) {
                /* ctrl-c at the other end */
                break;
            }

            net2tap++;

            /* read packet */
            nread = read_n(net_fd, buffer, ntohs(plength));
            printf("net2tap %lu: read %d bytes from the network\n", net2tap, nread);

            /* now buffer[] contains a full packet or frame, write it into the tun/tap interface */
            nwrite = cwrite(tap_fd, buffer, nread);
            printf("net2tap %lu: written %d bytes to the tap interface\n", net2tap, nwrite);
        }
    }
}

/*
 * reads from the port and writes to the tap device
 */
int Server(char **argv){
	sockaddr_in sockAddr, remote; 
	int serverSocket, connection; 
	char buffer[BUFSIZE]; 
	boolean binder, listener; 
	int port; 
    //tun variables
    int maxfd;
    int tap_fd;
    int sock_fd, net_fd, optval = 1;
    uint16_t nread, nwrite, plength;
    unsigned long int tap2net = 0, net2tap = 0;
    int16_t typeMes;
    char *if_name = (char *)calloc(100, sizeof(char));
    //test
    socklen_t remotelen;
	
	port = atoi(argv[1]); 
	strcpy(if_name, argv[2]);
	
	/*create the socket*/
	serverSocket = socket(AF_INET, SOCK_STREAM, 0); 
	if(serverSocket < 0){ 
		printf("Failed to create the socket");
		return 1; 
	}

	/*set the port for the socket*/
	memset(&sockAddr, 0, sizeof(sockAddr)); 
	sockAddr.sin_family = AF_INET; 
	sockAddr.sin_addr.s_addr = htonl (INADDR_ANY);
	sockAddr.sin_port = htons(port);
	
	/*bind the socket to the port*/
	binder = bind(serverSocket, (sockaddr *) &sockAddr, sizeof(sockAddr));
	if(binder){
		printf("Failed to bind to port\n"); 
		close(serverSocket);
		return 1; 
	}

	/*set the amount of connections in the queue for the socket*/
	listener = listen(serverSocket, 5); 
	if(listener < 0){ 
		printf("Failed to listen to port");
		close(serverSocket); 
		return 1; 
	}

    /* wait for connection request */
    remotelen = sizeof(remote);
    memset(&remote, 0, remotelen);
    if ((net_fd = accept(serverSocket, (struct sockaddr*)&remote, &remotelen)) < 0) {
        perror("accept()");
        exit(1);
    }


    /* Connecting to tap. now you can read/write on tap_fd (used combo of tunnel function 
        from assignment pdf + simpletun c file */ 
    if ( (tap_fd = allocate_tunnel(if_name, IFF_TAP | IFF_NO_PI)) < 0 ) {
        perror("Opening tap interface failed! \n");
        exit(1);
    }

    /* use select() to handle two descriptors at once */
    maxfd = (tap_fd > net_fd)?tap_fd:net_fd;

	/*get incoming connection*/
	while(1){
		//connection = accept(serverSocket, (sockaddr*)NULL, NULL);
/*
		if(connection <= 0){ 
			printf("Error accepting client connection\n"); 
			close(serverSocket); 	
			return 1; 
		}
		*/

		printf("got a conn!\n");

        int ret;
        fd_set rd_set;

        FD_ZERO(&rd_set);
        FD_SET(tap_fd, &rd_set); FD_SET(net_fd, &rd_set);

        ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);

        if (ret < 0 && errno == EINTR){
            continue;
        }

        if (ret < 0) {
            perror("select()");
            exit(1);
        }

        if(FD_ISSET(net_fd, &rd_set)) {
            /* data from the network: read it, and write it to the tun/tap interface. 
            * we need to read the length first, and then the packet */
			printf("SSERVER:::reading from the network, writing to tap");
            /* read length */
            nread = read_n(net_fd, (char *)&plength, sizeof(plength));
            if(nread == 0) {
                /* ctrl-c at the other end */
                break;
            }

            net2tap++;

            /* read packet */
            nread = read_n(net_fd, buffer, ntohs(plength));
            printf("net2tap %lu: read %d bytes from the network\n", net2tap, nread);

            /* now buffer[] contains a full packet or frame, write it into the tun/tap interface */
            nwrite = cwrite(tap_fd, buffer, nread);
            printf("net2tap %lu: written %d bytes to the tap interface\n", net2tap, nwrite);
        }


        if(FD_ISSET(tap_fd, &rd_set)) {
            /* data from tun/tap: just read it and write it to the network */
			printf("SERVER::Jreading from the tap, writing to network");
            nread = cread(tap_fd, buffer, BUFSIZE);

            tap2net++;
            printf("TAP2NET %lu: Read %d bytes from the tap interface\n", tap2net, nread);

            /* write length + packet */
            plength = htons(nread);
            nwrite = cwrite(net_fd, (char *)&plength, sizeof(plength));
            nwrite = cwrite(net_fd, buffer, nread);

            printf("TAP2NET %lu: Written %d bytes to the network\n", tap2net, nwrite);
        }

		close(connection);
		sleep(1);
	}
}
