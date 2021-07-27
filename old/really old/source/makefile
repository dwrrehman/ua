# Make file for the brain.

# Name
EXEC = universe
BASE = ./source/


CC_CPP = clang++ -std=c++17
CC_C = clang

INCLUDES = -I$(BASE)
CC_FLAGS = -Wall $(INCLUDES) -Wno-unused-command-line-argument
MY_LIBS = -pthread -lpthread

CPP_SOURCES = $(shell find $(BASE) -name "*.cpp")
C_SOURCES = $(shell find $(BASE) -name "*.c")

CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)
C_OBJECTS = $(C_SOURCES:.c=.o)

$(EXEC): $(CPP_OBJECTS) $(C_OBJECTS)
	$(CC_CPP) $(CPP_OBJECTS) $(C_OBJECTS) -o $(EXEC) $(MY_LIBS)


%.o: %.c
	$(CC_C)   -c $< -o $@ $(CC_FLAGS) $(MY_LIBS) 


%.o: %.cpp
	$(CC_CPP) -c $< -o $@ $(CC_FLAGS) $(MY_LIBS) 


clean:
	rm -f $(EXEC) $(C_OBJECTS) $(CPP_OBJECTS)

new:
	$(MAKE) clean
	$(MAKE) $(EXEC)


