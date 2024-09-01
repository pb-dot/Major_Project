################### How to use this make file ###########
## option1 $$ make -j4  			(builds .o and .exe in 4 parallel jobs)
## option2 $$ make debug 			(builds .o and .exe with -g flag, .exe can be used by gdb)
## option3 $$ make clean 			(removes all .o)
## option4 $$ make clean_all 		(removes .o and .exe)


# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
DEBUGFLAGS = -g

# Directories
LB_SRC = .
HEADERS_DIR = $(LB_SRC)/Headers
HELPERS_DIR = $(LB_SRC)/Helper
THREADS_DIR = $(LB_SRC)/Threads

# Add any new directories here as needed
CPP_DIRS = $(HELPERS_DIR) $(THREADS_DIR)

# Collect all .cpp files with full path
SRCS = $(foreach dir, $(CPP_DIRS), $(wildcard $(dir)/*.cpp)) $(LB_SRC)/main.cpp

# Object files with the same directory structure
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

# Output executable
TARGET = lb_exe

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to generate .o files (Note -I flag tells the compiler to look for header files in this directory)
$(LB_SRC)/%.o: $(LB_SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(HEADERS_DIR) -c "$<" -o "$@"

# Clean rule to remove intermediate files
clean:
	rm -f $(OBJS)

clean_all:
	rm -f $(OBJS) $(TARGET)

# Rule to build with debugging information
debug: clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(DEBUGFLAGS)" $(TARGET)

.PHONY: all clean clean_all debug

