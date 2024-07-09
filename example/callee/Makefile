CXX = g++
CXXFLAGS = -std=c++11 -Wall -g
LDFLAGS = -L/usr/local/lib -lmprpc -lmuduo_net -lmuduo_base -lprotobuf -lpthread

SRC = friendservice.cc mprpcapplication.cpp rpcprovider.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = friendservice

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
