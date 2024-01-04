#include <jni.h>
#include <liburing.h>
#include <iostream>
#include <arpa/inet.h>
#include "hooks.h"

struct io_uring ring{};

JNIEXPORT jint JNICALL Java_me_purp_mynt_Hooks_setupRing(JNIEnv* env, jobject object, jint value) {
    return io_uring_queue_init_params(value, &ring, 0);
    io_uring_peek_cqe()
}

const int OP_CONNECT = 0;

struct Connection {
    int type;
    int socket;
};

JNIEXPORT jint JNICALL Java_me_purp_mynt_Hooks_connect(JNIEnv* env, jobject obj, jstring string, jint port) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    if (!sqe) return -1;
    const auto fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) return fd;

    struct sockaddr_in target {};
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    const char* address = env->GetStringUTFChars(string, nullptr);
    if (address == nullptr) {
        close(fd);
        return -1;
    }

    if (inet_pton(AF_INET, address, &target.sin_addr) <= 0) {
        close(fd);
        env->ReleaseStringUTFChars(string, address);
        return -1;
    }

    env->ReleaseStringUTFChars(string, address);

    auto connection = Connection { OP_CONNECT, fd };
    io_uring_prep_connect(sqe, fd, (struct sockaddr*)&target, sizeof(target));
    io_uring_sqe_set_data(sqe, &connection);
    const auto submission = io_uring_submit(&ring);
    if (submission < 0) {
        close(fd);
        return submission;
    }

    struct io_uring_cqe* cqe;
    while (true) {
        if (io_uring_wait_cqe(&ring, &cqe) < 0) {
            close(fd);
            return -1;
        }

        auto data = (Connection*) io_uring_cqe_get_data(cqe);
        if (data->type == OP_CONNECT && fd == data->socket) {
            io_uring_cqe_seen(&ring, cqe);
            if (cqe->res < 0) {
                close(fd);
                return -1;
            }
            break;
        }
    }

    return fd;
}

JNIEXPORT jint JNICALL Java_me_purp_mynt_Hooks_accept(JNIEnv* env, jobject address, jint port) {
    return 0;
}

JNIEXPORT jint JNICALL Java_me_purp_mynt_Hooks_close(JNIEnv* env, jobject object, jint fd) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    if (!sqe) return -1;
    io_uring_prep_close(sqe, fd);
    const auto submission = io_uring_submit(&ring);
    if (submission < 0) {
        close(fd);
        return submission;
    }

    return 0;
}

//#define QUEUE_DEPTH 16
//
//int main(int size, char* args[]) {
//    struct io_uring ring{};
//    int file = open("/home/ubuntu/projects/test/build/files/example", O_RDONLY);
//    if (file < 0) {
//        std::cerr << "Error opening file." << std::endl;
//        return 1;
//    }
//
//    char buffer[1024];
//    struct iovec iov {};
//    iov.iov_base = buffer;
//    iov.iov_len = sizeof(buffer);
//    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
//    io_uring_prep_readv(sqe, file, &iov, 1, 0);
//
//    io_uring_prep_connect()
//
//    if (io_uring_submit(&ring) < 0) {
//        std::cerr << "Error submitting read operation." << std::endl;
//        return 1;
//    }
//
//    struct io_uring_cqe* cqe;
//    if (io_uring_wait_cqe(&ring, &cqe) < 0) {
//        std::cerr << "Error waiting for completion." << std::endl;
//        return 1;
//    }
//
//    if (cqe->res >= 0) {
//        std::cout << "Read completed successfully. Data: " << buffer << std::endl;
//    } else {
//        std::cerr << "Read failed with error: " << cqe->res << std::endl;
//    }
//
//    io_uring_cqe_seen(&ring, cqe);
//    io_uring_queue_exit(&ring);
//    close(file);
//    return 0;
//}