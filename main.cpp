#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>


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

    exit(EXIT_SUCCESS);
}
