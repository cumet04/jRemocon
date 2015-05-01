

make_all: work/lirc_input work/width_array work/signal_string

work/lirc_input: lirc_input.cpp
	g++ -std=c++11 -o $@ $<

work/width_array: width_array.cpp
	g++ -std=c++11 -o $@ $<

work/signal_string: signal_string.cpp
	g++ -std=c++11 -o $@ $<

# 
# TARGET = signal_analyzer
# SRCS = $(shell ls *.cpp)
# OBJS = $(SRCS:.cpp=.o)
# 
# work/$(TARGET): $(patsubst %,work/%,$(OBJS))
# 	g++ -o $@ $^
# 
# work/%.o : %.cpp
# 	g++ -std=c++11 -c -o $@ $<
# 
# clean:
# 	rm work/*
