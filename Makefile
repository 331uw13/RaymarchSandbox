FLAGS = -ggdb -Wall -Wextra
CXX = g++

IMGUIDIR = imgui
TARGET_NAME = rmsb


SRC  = $(shell find ./src -type f -name *.cpp)
SRC += $(IMGUIDIR)/imgui.cpp $(IMGUIDIR)/imgui_demo.cpp $(IMGUIDIR)/imgui_draw.cpp $(IMGUIDIR)/imgui_tables.cpp $(IMGUIDIR)/imgui_widgets.cpp
SRC += $(IMGUIDIR)/backends/imgui_impl_opengl3.cpp $(IMGUIDIR)/backends/imgui_impl_glfw.cpp

LIBS = -lraylib -lGL -lm -lpthread -lX11 -ldl -lrt -lglfw 
OBJS = $(SRC:.cpp=.o)

all: $(TARGET_NAME)


%.o: %.cpp
	@echo "compile"
	$(CXX) $(FLAGS) -I$(IMGUIDIR) -I$(IMGUIDIR)/backends -c $< -o $@ 

$(TARGET_NAME): $(OBJS)
	@echo "link"
	$(CXX) $(OBJS) -o $@ $(LIBS) $(FLAGS)

clean:
	rm $(OBJS) $(TARGET_NAME)

.PHONY: all clean

