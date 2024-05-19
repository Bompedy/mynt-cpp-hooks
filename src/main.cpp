#include <jni.h>
#include <iostream>
#include <netinet/in.h>
#include <bits/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "hooks.h"
#include <map>

#define ERROR_NONE (-1)
#define ERROR_CREATE_SOCKET 0
#define ERROR_SET_NON_BLOCKING 1
#define ERROR_CREATE_CONNECTION 2
#define ERROR_BIND_SOCKET 3
#define ERROR_LISTEN_SOCKET 4

std::map<sockaddr_in, int> servers;

JNIEXPORT void JNICALL Java_com_github_mynt_Hooks_connect(
        JNIEnv *env,
        jobject hooks,
        jint epoll,
        jobject provider,
        jobject uuid,
        jstring address,
        jint port
) {
    jclass cls = env->FindClass("com/github/mynt/Provider");
    jmethodID method = env->GetMethodID(cls, "onConnected", "(Ljava/util/UUID;III)V");

    const char *str_address = env->GetStringUTFChars(address, JNI_FALSE);
    sockaddr_in sock_addr{};
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    inet_pton(AF_INET, str_address, &sock_addr.sin_addr);
    env->ReleaseStringUTFChars(address, str_address);

    jint client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0) {
        env->CallVoidMethod(provider, method, uuid, client, client, ERROR_CREATE_SOCKET);
        return;
    }

    auto connection = connect(client, (sockaddr*) &sock_addr, sizeof(sock_addr));
    if (connection < 0) {
        env->CallVoidMethod(provider, method, uuid, client, connection, ERROR_CREATE_CONNECTION);
        close(client);
        return;
    }

    const auto flags = fcntl(client, F_GETFL, 0);
    if (flags < 0) {
        env->CallVoidMethod(provider, method, uuid, client, flags, ERROR_SET_NON_BLOCKING);
        close(client);
        return;
    }

    const auto set_flags = fcntl(client, F_SETFL, flags | O_NONBLOCK);
    if (set_flags < 0) {
        env->CallVoidMethod(provider, method, uuid, client, set_flags, ERROR_SET_NON_BLOCKING);
        close(client);
        return;
    }

    env->CallVoidMethod(provider, method, uuid, client, 0, ERROR_NONE);
}

JNIEXPORT void JNICALL Java_com_github_mynt_Hooks_accept(
        JNIEnv *env,
        jobject hooks,
        jint epoll,
        jobject provider,
        jobject uuid,
        jstring address,
        jint port
) {
    jclass cls = env->FindClass("com/github/mynt/Provider");
    jmethodID method = env->GetMethodID(cls, "onConnected", "(Ljava/lang/Object;Ljava/lang/Object;I;I;I)V");

    const char *str_address = env->GetStringUTFChars(address, JNI_FALSE);
    sockaddr_in sock_addr{};
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    inet_pton(AF_INET, str_address, &sock_addr.sin_addr);
    env->ReleaseStringUTFChars(address, str_address);

    const auto server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        env->CallVoidMethod(provider, method, uuid, server, server, ERROR_CREATE_SOCKET);
        return;
    }

    const auto flags = fcntl(server, F_GETFL, 0);
    if (flags < 0) {
        env->CallVoidMethod(provider, method, uuid, server, flags, ERROR_SET_NON_BLOCKING);
        close(server);
        return;
    }

    const auto set_flags = fcntl(server, F_SETFL, flags | O_NONBLOCK);
    if (set_flags < 0) {
        env->CallVoidMethod(provider, method, uuid, server, set_flags, ERROR_SET_NON_BLOCKING);
        close(server);
        return;
    }

    const auto bind_result = bind(server, (sockaddr*) &sock_addr, sizeof(sock_addr));
    if (bind_result < 0) {
        env->CallVoidMethod(provider, method, uuid, server, bind_result, ERROR_BIND_SOCKET);
        close(server);
        return;
    }

    const auto listen_result = listen(server, SOMAXCONN);
    if (listen_result < 0) {
        env->CallVoidMethod(provider, method, uuid, server, listen_result, ERROR_LISTEN_SOCKET);
        close(server);
        return;
    }
}

JNIEXPORT void JNICALL Java_com_github_mynt_Hooks_close(
        JNIEnv *env,
        jobject hooks,
        jint epoll,
        jint socket
) {
    epoll_ctl(epoll, EPOLL_CTL_DEL, socket, nullptr);
    close(socket);
}

JNIEXPORT jboolean JNICALL Java_com_github_mynt_Hooks_read(
        JNIEnv *env,
        jobject hooks,
        jint epoll,
        jobject connection,
        jint socket,
        jlong from,
        jlong to,
        jlong end
) {
    struct epoll_event read_event{};
    read_event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    read_event.data.fd = socket;
    epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &read_event);
    const auto wait = epoll_wait(epoll, &read_event, 1, -1);
    if (wait != 1) {
        std::cout << "PROBLEM READING!!!" << std::endl;
        return false;
    }

    char* buffer = reinterpret_cast<char*>(from);
    long next = from;
    while (next < to) {
        auto amount = read(socket, buffer, end - (to - next));
        next = from + amount;
        buffer += next;

        if (errno == EAGAIN || errno == EWOULDBLOCK) {

        }
    }


    return true;
}

JNIEXPORT jboolean JNICALL Java_com_github_mynt_Hooks_write(
        JNIEnv *env,
        jobject hooks,
        jint epoll,
        jobject connection,
        jint socket,
        jlong from,
        jlong to
) {
    std::cout << "write" << std::endl;

    return true;
}

JNIEXPORT jint JNICALL Java_com_github_mynt_Hooks_epoll(
        JNIEnv *env,
        jobject clazz
) {
    jint epoll = epoll_create1(0);
    return epoll;
}
//
//
//int main(int size, char* args[]) {
//    const auto epoll = guard(epoll_create1(0), "cannot create epoll");
//
//    sockaddr_in address{};
//    address.sin_family = AF_INET;
//    address.sin_port = htons(6964);
//    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
//    std::thread thread([&]() {client(address);});
//    thread.detach();
//
//    server = guard(socket(AF_INET, SOCK_STREAM, 0), "cannot create server socket");
//    set_nonblocking(server);
//    guard(bind(server, (sockaddr*) &address, sizeof(address)), "Error binding");
//    guard(listen(server, SOMAXCONN), "Error listening");
//
//    struct epoll_event accept_event{};
//    accept_event.events = EPOLLIN;
//    accept_event.data.fd = server;
//    guard(epoll_ctl(epoll, EPOLL_CTL_ADD, server, &accept_event), "cannot setup server for listening");
//
//
//    struct epoll_event events[MAX_EVENTS];
//    for (;;) {
//        const auto count = guard(epoll_wait(epoll, events, MAX_EVENTS, -1), "cannot wait");
//        for (int i = 0; i < count; ++i) {
//            const auto triggered = events[i];
//            // accept connection
//            if (triggered.data.fd == server) {
//                sockaddr_in client_address{};
//                socklen_t address_len = sizeof(client_address);
//                const auto client = guard(accept(server, (sockaddr*) &client_address, &address_len), "cannot accept");
//                std::cout << "we got a client: " << client << std::endl;
//                set_nonblocking(client);
//                struct epoll_event client_event{};
//                client_event.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLONESHOT;
//                client_event.data.fd = client;
//                guard(epoll_ctl(epoll, EPOLL_CTL_ADD, client, &client_event), "cannot add client event");
//            } else {
//                if (triggered.events & EPOLLIN) {
//                    char buffer[1];
//                    int total = 0;
//                    while (true) {
//                        auto i = read(triggered.data.fd, buffer, 1);
//                        total += i;
//                        std::cout << "got " << i << " bytes!" << std::endl;
//                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                            std::cout << "yea done reading" << std::endl;
//                        }
//                        //
//                        if (i <= 0) break;
//                    }
//
//                    std::cout << "REad a total of " << total << " bytes" << std::endl;
//
//                    close(triggered.data.fd);
//                    close(server);
//                }
//                if (triggered.events & EPOLLOUT) {
//                    std::cout << "ready to write: " << triggered.data.fd << std::endl;
//                }
//            }
//        }
//    }

//        thread.join();
//    return 0;
//}