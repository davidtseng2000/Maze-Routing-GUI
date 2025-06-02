CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -IC:/SFML-2.5.1/include
LDFLAGS = -LC:/SFML-2.5.1/lib -lsfml-graphics -lsfml-window -lsfml-system

OBJS = main.o utils.o objects.o draw.o
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

main.o: main.cpp utils.h objects.h draw.h
	$(CXX) $(CXXFLAGS) -c main.cpp

utils.o: utils.cpp utils.h objects.h
	$(CXX) $(CXXFLAGS) -c utils.cpp

objects.o: objects.cpp objects.h
	$(CXX) $(CXXFLAGS) -c objects.cpp

draw.o: draw.cpp draw.h
	$(CXX) $(CXXFLAGS) -c draw.cpp

clean:
	rm -f $(OBJS) $(TARGET)
