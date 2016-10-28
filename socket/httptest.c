#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>

#define HOST "127.0.0.1"
#define PORT 10240
#define MAX_SEND_SIZE 4096

int main(int argc, const char *argv[]) {
    int serverfd;
    struct sockaddr_in server;
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    char buffer[MAX_SEND_SIZE];
    if (serverfd < 0) {
        printf("open socket error: %s", strerror(errno));
        exit(1);
    }
    bzero((char *)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");  ///服务器ip
    if (connect(serverfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("connect server error: %s", strerror(errno));
        exit(1);
    }
    buffer[0] = 'c';
    write(serverfd, buffer, strlen(buffer));
    return 0;
}
