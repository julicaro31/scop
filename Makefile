CXX = c++
CC  = cc

CXXFLAGS = -Wall -Wextra -Werror -Iinclude -Iinclude/math -Iinclude/graphics -Iinclude/parser
CFLAGS   = -Wall -Wextra -Werror -Iinclude

LIBS = -lGL -lglfw -ldl -lX11 -lpthread -lXrandr -lXi

CPP_SRCS = src/main.cpp src/graphics/shader.cpp

C_SRCS   = src/glad.c

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