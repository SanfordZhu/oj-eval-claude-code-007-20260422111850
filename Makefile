CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Source files
SOURCES = Basic/Basic.cpp \
          Basic/evalstate.cpp \
          Basic/exp.cpp \
          Basic/parser.cpp \
          Basic/program.cpp \
          Basic/statement.cpp \
          Basic/Utils/error.cpp \
          Basic/Utils/strlib.cpp \
          Basic/Utils/tokenScanner.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Target executable
TARGET = code

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean