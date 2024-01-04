#include <jni.h>
#include <iostream>
#include "hooks.h"

JNIEXPORT void JNICALL Java_me_lucas_mynt_Test_printHello(JNIEnv *, jobject) {
    std::cout << "Hello from C++!" << std::endl;
}