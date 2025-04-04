CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17

NAME		= webserv
OBJ_DIR		= obj
SRC_DIR		= src
INC_DIR		= include
REMOVE		= rm -rf

# INTERNAL FUNCTIONS
FUNCTIONS	=	$(SRC_DIR)/main.cpp \
				$(SRC_DIR)/SimpleServer.cpp \
				$(SRC_DIR)/SimpleServer_Launch.cpp \
				$(SRC_DIR)/SimpleServer_test.cpp \
				$(SRC_DIR)/SimpleServer_Configuration.cpp \
				$(SRC_DIR)/HttpRequest_Helper.cpp \
				$(SRC_DIR)/HttpRequest_Debugger.cpp \
				$(SRC_DIR)/HttpRequest_GetterSetter.cpp \
				$(SRC_DIR)/HttpRequest_ParsingTokenizer.cpp \
				$(SRC_DIR)/HttpRequest_Handler.cpp \
				$(SRC_DIR)/ServerConfig.cpp \


# INTERNAL OBJECT
OBJECTS		= $(addprefix $(OBJ_DIR)/, $(notdir $(FUNCTIONS:.cpp=.o)))

all : $(NAME)

# INTERNAL RULE
$(NAME) : $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)

# DIRECTORY
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	$(REMOVE) $(OBJECTS)
	$(REMOVE) $(OBJ_DIR)

fclean : clean
	$(REMOVE) $(NAME)

re : fclean all

.PHONY : all clean fclean re