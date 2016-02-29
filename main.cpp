#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>

#include <thread>
#include <stdlib.h>

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

//ToDo realize fast and threadsafe log
void log (const char* format,...){
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}
//-----------------------------------------------------------------------------
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
//------------------------------------------------------------------------------

int demonize (){
    int pid = fork();

    switch(pid) {
    case 0:

    case -1:
        log("Error: unable to fork\n");
        break;

    default:
        log("Success: process %d went to background\n", pid);
        setsid();

        close(0);
        close(1);
        close(2);
        exit(EXIT_SUCCESS);
        break;
    }

    return 0;
}

int openSocket(const std::string &tcp_ip_v4_addres, unsigned int port ){
    sockaddr_in server;
    //Create a socket
    int serverSocket = INVALID_SOCKET;
    if((serverSocket = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
        ThrowRuntime ( "Could not create socket : %d" , errNum() );

    log("Socket created.\n");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( port );    //ToDo Port here

    //Bind
    if( bind(serverSocket ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
        ThrowRuntime("Bind failed with error code : %d" , errNum());

    puts("Bind done");
    return serverSocket;
}

//------------------------------------------------------------------------------
void processHttpReq (const std::string &req,int socket){
    auto lines = split (req,'\x0A');
    auto header = split (lines[0],' ');

    if (header[0] == "GET"){
        log("GET req\n");
        auto resName = header[1];

        if (resName == "/")
            resName = "/index.html";

        if (resName[0] == '/')
            resName = std::string(&resName[1]);
        resName = resName.substr(0,resName.find('?',0));

        auto f = fopen(resName.c_str(),"rb");
        if (f){
            fseek(f,0,SEEK_END);
            auto size = ftell(f);
            fseek(f,0,SEEK_SET);

            std::vector<char> buff;
            buff.resize(size);
            fread(buff.data(),size,1,f);
            fclose(f);

            std::string header = std::string("HTTP/1.0 200 OK\x0D\x0A") +
                                 "Content-Length :"+ std::to_string(buff.size()) +"\x0D\x0A" +
                                 "Connection:keep-alive\x0D\x0A" +
                                 "\x0D\x0A";
            write(socket,header.c_str(),header.size());


            write(socket,buff.data(),buff.size());


            log("Resource %s is sent.\n",resName.c_str());
        }
        else{
            log("Resource %s is not found.\n",resName.c_str());
            char respound[] = "HTTP/1.0 404 NOT FOUND\x0D\x0AContent-Type: text/html\x0D\x0A\x0D\x0A";
            write(socket,respound,sizeof(respound));
        }
    }
    else
        log("Unsuported req\n");

}

void processConection (int socket){

    unsigned int conectionId = rand();
    log("Open connection %i\n",conectionId);

    char buffer[10*1024];

    while (true){
        auto size = read (socket,&buffer,10*1024);
        if (size <= 0)
            break;

        log("New reqwest in connection %i \n",conectionId);
        processHttpReq (buffer,socket);
        fflush(stdout);

    }

    close (socket);
    log("Close connection %i\n",conectionId);
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
            log("%s\n",optarg);
            break;    
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    demonize ();

    chdir(dir.c_str());

    auto serverSocket = openSocket("", std::stoi(port));
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
