#ifndef SHROUD_H
#define SHROUD_H

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    typedef SOCKET socket_t;
    #define GET_ERROR() WSAGetLastError()
    #define CLOSE_SOCKET(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    #include <pthread.h> // Added missing pthread header
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define GET_ERROR() errno
    #define CLOSE_SOCKET(s) close(s)
#endif

// Static inline prevents "multiple definition" errors
static inline int init_sockets() {
#ifdef _WIN32
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
    return 0;
#endif
}

static inline void cleanup_sockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

// Threading Abstraction
typedef struct {
#ifdef _WIN32
    HANDLE handle;
#else
    pthread_t handle;
#endif
} thread_t;

typedef void* (*thread_func_t)(void*);

static inline int create_thread(thread_t *thread, thread_func_t func, void *arg) {
#ifdef _WIN32
    thread->handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL);
    return (thread->handle == NULL) ? -1 : 0;
#else
    return (pthread_create(&thread->handle, NULL, func, arg) != 0) ? -1 : 0;
#endif
}

static inline int join_thread(thread_t *thread, void **retval) {
#ifdef _WIN32
    return (WaitForSingleObject(thread->handle, INFINITE) == WAIT_FAILED) ? -1 : 0;
#else
    return (pthread_join(thread->handle, retval) != 0) ? -1 : 0;
#endif
}

static inline void destroy_thread(thread_t *thread) {
#ifdef _WIN32
    if (thread->handle) CloseHandle(thread->handle);
#endif
}

#endif