CXX = clang++
CXXFLAGS = -std=c++20

#jni
JNI_INCLUDE_PATH = /home/ubuntu/.jdks/azul-17.0.9/include
JNI_INCLUDE_PATH2 = /home/ubuntu/.jdks/azul-17.0.9/include/linux
#jni

#iouring
LIBURING_INCLUDE_DIR = /usr/include
LIBURING_LIB_DIR = /usr/lib/x86_64-linux-gnu/liburing.so
LIBURING_LIB = -luring
#iouring

SRCS = src/main.cpp
LIBRARY_TARGET = /home/ubuntu/projects/mynt-iouring/libmynt-hooks.so
EXECUTABLE_TARGET = build/libmynt-hooks

all: clean $(LIBRARY_TARGET)
#$(EXECUTABLE_TARGET)

$(LIBRARY_TARGET): $(SRCS) src/hooks.h
	$(CXX) $(CXXFLAGS) -shared -fPIC -o $(LIBRARY_TARGET) $(SRCS) -I$(JNI_INCLUDE_PATH) -I$(JNI_INCLUDE_PATH2) -I$(LIBURING_INCLUDE_DIR) -L$(LIBURING_LIB_DIR) $(LIBURING_LIB)

$(EXECUTABLE_TARGET): $(SRCS) src/hooks.h
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE_TARGET) $(SRCS) -I$(JNI_INCLUDE_PATH) -I$(JNI_INCLUDE_PATH2) -I$(LIBURING_INCLUDE_DIR) -L$(LIBURING_LIB_DIR) $(LIBURING_LIB)

library: $(LIBRARY_TARGET)

executable: $(EXECUTABLE_TARGET)
	./$(EXECUTABLE_TARGET)

clean:
	rm -f $(LIBRARY_TARGET) $(EXECUTABLE_TARGET)