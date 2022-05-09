CXX = g++
CXXFLAGS += -std=c++17 -Wall -pedantic -Wextra -Wno-unused-variable
SOURCES = main.cpp InitialGenerator.cpp ../MainClasses.h ../TinyXml/tinystr.cpp ../TinyXml/tinyxml.cpp ../TinyXml/tinyxmlerror.cpp ../TinyXml/tinyxmlparser.cpp 
OBJECTS = $(SOURCES:.cpp=.o) 
EXECUTABLE = main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) *.h.gch