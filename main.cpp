#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>

#include <thread>

#ifdef WIN32
#include <winsock2.h>
#define errNum WSAGetLastError
typedef  ssize_t socklen_t;
#define ioctl  ioctlsocket
#endif

#ifdef UNIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <sys/ioctl.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
int errNum (){ return errno;}
#define closesocket close
#endif

#include <cstdio>
#include <stdarg.h>

template <typename Error>
void Throw(const char* format,...){

	va_list ap;
	char buff[256];
	va_start(ap, format);
	vsprintf(buff, format, ap);
	va_end(ap);

        throw Error (buff);
}
#define ThrowRuntime Throw<std::runtime_error>


int demonize (){
    int pid = fork();

    switch(pid) {
	case 0:
	    setsid();
	    chdir("/");

	    close(0);
	    close(1);
	    close(2);

	    exit(EXIT_SUCCESS);

	case -1:
	    printf("Error: unable to fork\n");
	    break;

	default:
	    printf("Success: process %d went to background\n", pid);
	    break;
    }

    return 0;
}

int openSocket(const std::string &tcp_ip_v4_addres){
    sockaddr_in server;
    //Create a socket
    int serverSocket = INVALID_SOCKET;
    if((serverSocket = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
        ThrowRuntime ( "Could not create socket : %d" , errNum() );

    printf("Socket created.\n");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8080 );    //ToDo Port here

    //Bind
    if( bind(serverSocket ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
        ThrowRuntime("Bind failed with error code : %d" , errNum());

    puts("Bind done");
    return serverSocket;
}

void processConection (int socket){
    //printf("new thread\n");
    close (socket);
}

int main(int argc, char *argv[])
{
    std::string ip;
    std::string port;
    std::string dir;

    int  opt;
    while ((opt = getopt(argc, argv, "h:p:d:")) != -1) {
        switch (opt) {
        case 'h':
            ip = optarg;
        case 'p':  
            port = optarg;
        case 'd':
            dir = optarg;
            printf("%s\n",optarg);
            break;    
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    demonize ();

    auto serverSocket = openSocket("");
    while (1){
        if( 0 != listen(serverSocket, 5))
            ThrowRuntime("Listen failed with error code : %d", errNum());

        int newsock = INVALID_SOCKET;
        sockaddr_in  cli_addr;
        socklen_t cli_len;
        if (INVALID_SOCKET == (newsock = accept(serverSocket, (struct sockaddr *) &cli_addr, &cli_len)) )
            ThrowRuntime("accept() failed: %d\n", errno);

        //printf("income from port %i\n",cli_addr.sin_port);

        std::thread thread (processConection,newsock);
        thread.detach();
    }

    closesocket(serverSocket);

    exit(EXIT_SUCCESS);
}
