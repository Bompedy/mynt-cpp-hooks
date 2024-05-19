CXX = clang++
CXXFLAGS = -std=c++20

#jni
JNI_INCLUDE_PATH = /home/mason/.jdks/zulu17.50.19-ca-jdk17.0.11-linux_x64/include
JNI_INCLUDE_PATH2 = /home/mason/.jdks/zulu17.50.19-ca-jdk17.0.11-linux_x64/include/linux
#jni

SRCS = src/main.cpp
LIBRARY_TARGET = /home/mason/kotlin/kt-iouring/libmynt-hooks.so
EXECUTABLE_TARGET = build/libmynt-hooks

all: clean $(LIBRARY_TARGET)
#$(EXECUTABLE_TARGET)

$(LIBRARY_TARGET): $(SRCS) src/hooks.h
	$(CXX) $(CXXFLAGS) -shared -fPIC -o $(LIBRARY_TARGET) $(SRCS) -I$(JNI_INCLUDE_PATH) -I$(JNI_INCLUDE_PATH2)

$(EXECUTABLE_TARGET): $(SRCS) src/hooks.h
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE_TARGET) $(SRCS) -I$(JNI_INCLUDE_PATH) -I$(JNI_INCLUDE_PATH2)

library: $(LIBRARY_TARGET)

executable: $(EXECUTABLE_TARGET)
	./$(EXECUTABLE_TARGET)

clean:
	rm -f $(LIBRARY_TARGET) $(EXECUTABLE_TARGET)