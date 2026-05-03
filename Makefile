CXX = c++
CC  = cc

UNAME_S := $(shell uname -s)

CXXFLAGS = -Wall -Wextra -Werror -Iinclude -Iinclude/math -Iinclude/graphics -Iinclude/parser
CFLAGS   = -Wall -Wextra -Werror -Iinclude

ifeq ($(UNAME_S),Darwin)
	GLFW_PREFIX := $(shell brew --prefix glfw 2>/dev/null)
	CXXFLAGS += -I$(GLFW_PREFIX)/include
	CFLAGS   += -I$(GLFW_PREFIX)/include
	LIBS = -L$(GLFW_PREFIX)/lib -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
	LIBS = -lGL -lglfw -ldl -lX11 -lpthread -lXrandr -lXi
endif

CPP_SRCS = src/main.cpp src/graphics/Shader.cpp src/graphics/Camera.cpp src/graphics/Renderer.cpp src/parser/ObjParser.cpp

C_SRCS   = src/glad/glad.c

CPP_OBJS = $(CPP_SRCS:.cpp=.o)
C_OBJS   = $(C_SRCS:.c=.o)

NAME = scop

all: $(NAME)

$(NAME): $(CPP_OBJS) $(C_OBJS)
	$(CXX) $(CPP_OBJS) $(C_OBJS) $(LIBS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CPP_OBJS) $(C_OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re