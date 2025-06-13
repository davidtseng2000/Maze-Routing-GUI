CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -IC:/SFML-2.5.1/include -IC:/gurobi1103/win64/include
LDFLAGS = -LC:/SFML-2.5.1/lib -LC:/gurobi1103/win64/lib -lsfml-graphics -lsfml-window -lsfml-system -lgurobi_c++mt -lgurobi110

OBJS = main.o utils.o objects.o draw.o ilp_solver.o
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

ilp_solver.o: ilp_solver.cpp ilp_solver.h
	$(CXX) $(CXXFLAGS) -c ilp_solver.cpp

clean:
	rm -f $(OBJS) $(TARGET)
