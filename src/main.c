#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

#include "shroud.h"

#define MAX_PORTS 100
#define TIMEOUT_SEC 2

typedef struct {
    socket_t s;
    int port;
} PortTarget;

// Upgraded Banner Grabber: Writes to both Console and Log File
void grab_banner(socket_t s, int port, FILE *log_file) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    
#ifdef _WIN32
    DWORD timeout = 1000; 
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif

    int bytes = recv(s, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        for(int i = 0; i < bytes; i++) {
            if(buffer[i] < 32 || buffer[i] > 126) buffer[i] = ' ';
        }
        printf("   [!] Banner: %s\n", buffer);
        if (log_file) fprintf(log_file, "   [Banner] %s\n", buffer);
    }
}

int set_non_blocking(socket_t s) {
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(s, FIONBIO, &mode);
#else
    int flags = fcntl(s, F_GETFL, 0);
    return fcntl(s, F_SETFL, flags | O_NONBLOCK);
#endif
}

void scan_ports(const char *target_ip, int start_port, int end_port, FILE *log_file) {
    PortTarget targets[MAX_PORTS];
    fd_set write_fds;
    struct timeval timeout;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, target_ip, &addr.sin_addr);

    for (int current = start_port; current <= end_port; ) {
        int count = 0;

        for (int i = 0; i < MAX_PORTS && (current <= end_port); i++, current++) {
            socket_t s = socket(AF_INET, SOCK_STREAM, 0);
            if (s == INVALID_SOCKET) continue;

            set_non_blocking(s);
            addr.sin_port = htons(current);
            connect(s, (struct sockaddr*)&addr, sizeof(addr));

            targets[count].s = s;
            targets[count].port = current;
            count++;
        }

        while (count > 0) {
            FD_ZERO(&write_fds);
            socket_t max_fd = 0;
            for (int i = 0; i < count; i++) {
                FD_SET(targets[i].s, &write_fds);
                if (targets[i].s > max_fd) max_fd = targets[i].s;
            }

            timeout.tv_sec = TIMEOUT_SEC;
            timeout.tv_usec = 0;

            if (select((int)max_fd + 1, NULL, &write_fds, NULL, &timeout) > 0) {
                for (int i = 0; i < count; i++) {
                    if (FD_ISSET(targets[i].s, &write_fds)) {
                        int error = 0;
                        socklen_t len = sizeof(error);
                        getsockopt(targets[i].s, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
                        
                        if (error == 0) {
                            printf("[+] Port %d is OPEN\n", targets[i].port);
                            if (log_file) fprintf(log_file, "Port %d: OPEN\n", targets[i].port);
                            grab_banner(targets[i].s, targets[i].port, log_file);
                        }
                        
                        CLOSE_SOCKET(targets[i].s);
                        targets[i] = targets[count - 1];
                        count--;
                        i--;
                    }
                }
            } else {
                for (int i = 0; i < count; i++) CLOSE_SOCKET(targets[i].s);
                count = 0;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <IP> <StartPort> <EndPort> [log_file.txt]\n", argv[0]);
        return 1;
    }

    const char *target_ip = argv[1];
    int start = atoi(argv[2]);
    int end = atoi(argv[3]);
    FILE *log_file = NULL;

    if (argc == 5) {
        log_file = fopen(argv[4], "a");
        if (!log_file) fprintf(stderr, "[-] Warning: Could not open log file.\n");
    }

    if (init_sockets() != 0) return 1;

    printf("GhostScanner starting on %s...\n", target_ip);
    if (log_file) fprintf(log_file, "\n--- Scan: %s (%d-%d) ---\n", target_ip, start, end);

    scan_ports(target_ip, start, end, log_file);

    if (log_file) {
        fprintf(log_file, "--- Scan Completed ---\n");
        fclose(log_file);
    }

    cleanup_sockets();
    return 0;
}