#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define MAX_CONFIG_SIZE 0x100 // 256
#define MAX_KEY_SIZE 0x80
#define MAX_VALUE_SIZE 0x80
#define MAX_RECV_SIZE 4096
#define PORT 10240

#define CONFIG_FILE "web.conf"

typedef struct hashtable{
    int exists;
    char *key;
    char *value;
} hashtable;

uint64_t hash(char *);
void __init(hashtable *config_table[]);
void __load_config(hashtable *config_table[]);
void __parse_config(char *, char *, char *);
char *get_config(hashtable *config_table[], char *);
void __start(hashtable *config_table[]);

int main(int argc, const char *argv[]) {
    uint64_t h = hash("abc\0");
    hashtable *config_table[MAX_CONFIG_SIZE];
    __init(config_table);
    __load_config(config_table);
    __start(config_table);
    return 0;
}

void
__start(hashtable *config_table[]) {
    /**
     * #include <netinet/in.h>
     * struct sockaddr_in {
     *     short            sin_family;   // e.g. AF_INET
     *     unsigned short   sin_port;     // e.g. htons(3490)
     *     struct in_addr   sin_addr;     // see struct in_addr, below
     *     char             sin_zero[8];  // zero this if you want to
     * };
     *
     * typedef uint32_t in_addr_t;
     * struct in_addr {
     *     in_addr_t s_addr;// load with inet_aton()
     * };
     *
     * struct sockaddr {
     *     unsigned  short  sa_family;     // address family, AF_xxx
     *     char  sa_data[14];                 // 14 bytes of protocol address
     * };
     */
    int serverfd;
    char buffer[MAX_RECV_SIZE];
    struct sockaddr_in server, client;

    // socket创建一个文件描述符,AF_INET是IPV4的协议,SOCK_STREAM是TCP协议
    serverfd = socket(AF_INET, SOCK_STREAM, 0); // 第一个参数必须要和server.sin_family的参数值相同它们才能匹配上,bind才不会报错
    if (serverfd < 0) {
        printf("open socket: %s", strerror(errno));
        exit(1);
    }

    // 初始化server的地址
    bzero((char *)&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htons(INADDR_ANY);
    server.sin_port = htons(PORT);

    int reuse = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));

    if (bind(serverfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("bind error: %s", strerror(errno));
        exit(1);
    }
    listen(serverfd, 5);
    socklen_t clientlen = sizeof(client);
    int clientfd = accept(serverfd, (struct sockaddr *)&client, &clientlen);
    if (clientfd < 0) {
        printf("accept error: %s", strerror(errno));
        exit(0);
    }
    read(clientfd, buffer, MAX_RECV_SIZE);
    printf("%s\n", buffer);
}

char *
get_config(hashtable *config_table[], char *key) {
    uint64_t h = hash(key) % MAX_CONFIG_SIZE;
    hashtable *config = config_table[h];
    if (config->exists) {
        return config->value;
    }
}

void
__load_config(hashtable *config_table[]) {
    FILE *fp = fopen(CONFIG_FILE, "r");
    char *line = (char *)malloc(sizeof(char) * (MAX_KEY_SIZE + MAX_VALUE_SIZE));
    memset(line, 0, MAX_KEY_SIZE + MAX_VALUE_SIZE);
    while (fgets(line, MAX_KEY_SIZE + MAX_VALUE_SIZE, fp) != NULL) {
        char *key = (char *)malloc(sizeof(char) * MAX_KEY_SIZE);
        char *value = (char *)malloc(sizeof(char) * MAX_VALUE_SIZE);
        __parse_config(line, key, value);
        uint64_t h = hash(key) % (MAX_CONFIG_SIZE);
        config_table[h]->exists = 1;
        config_table[h]->key = key;
        config_table[h]->value = value;
    }
    free(line);
}

void
__parse_config(char *line, char *key, char *value) {
    while ((*key++ = *line++) != '=');
    *(key - 1) = '\0';
    while ((*value++ = *line++) != '\n');
    *(value - 1)= '\0';
}

void
__init(hashtable *config_table[]) {
    int i = 0;
    for(; i < MAX_CONFIG_SIZE;) {
        hashtable *h = (hashtable *)malloc(sizeof(hashtable) * MAX_CONFIG_SIZE);
        h->exists = 0;
        h->key = (char *)malloc(sizeof(char) * MAX_KEY_SIZE);
        h->value = (char *)malloc(sizeof(char) * MAX_VALUE_SIZE);
        config_table[i++] = h;
    }
}

uint64_t
hash(char *key) {
    uint64_t hash = 0;
    while(*key) {
        hash += *key++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}
