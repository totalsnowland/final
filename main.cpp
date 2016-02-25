#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>

std::string ip;
std::string port;
std::string dir;

int main(int argc, char *argv[])
{
    int  opt;
    //int nsecs, tfnd;


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





    exit(EXIT_SUCCESS);
}