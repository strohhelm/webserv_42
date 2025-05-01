CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -g -O0 -fsanitize=address	
# -g -fsanitize=address	

NAME		= webserv
OBJ_DIR		= obj
SRC_DIR		= src
INC_DIR		= include
REMOVE		= rm -rf

# INTERNAL FUNCTIONS
FUNCTIONS	=	$(SRC_DIR)/CGI.cpp \
				$(SRC_DIR)/ConfigHelper.cpp \
				$(SRC_DIR)/ConfigMain.cpp \
				$(SRC_DIR)/ConfigRoute.cpp \
				$(SRC_DIR)/ConfigServer.cpp \
				$(SRC_DIR)/ConfigUtils.cpp \
				$(SRC_DIR)/HttpRequest_Debugger.cpp \
				$(SRC_DIR)/HttpRequest_GetterSetter.cpp \
				$(SRC_DIR)/HttpRequest_Handler_Get.cpp \
				$(SRC_DIR)/HttpRequest_Handler_Post_functions.cpp \
				$(SRC_DIR)/HttpRequest_Handler_Post.cpp \
				$(SRC_DIR)/HttpRequest_Handler_State.cpp \
				$(SRC_DIR)/HttpRequest_Handler.cpp \
				$(SRC_DIR)/HttpRequest_Helper.cpp \
				$(SRC_DIR)/HttpRequest_ParsingTokenizer.cpp \
				$(SRC_DIR)/HttpRequest_SendResponse.cpp \
				$(SRC_DIR)/HttpRequest_Validation.cpp \
				$(SRC_DIR)/main.cpp \
				$(SRC_DIR)/SimpleServer_Configuration.cpp \
				$(SRC_DIR)/SimpleServer_Launch.cpp \
				$(SRC_DIR)/SimpleServer.cpp
# $(SRC_DIR)/Post.cpp 



# INTERNAL OBJECT
OBJECTS		= $(addprefix $(OBJ_DIR)/, $(notdir $(FUNCTIONS:.cpp=.o)))

all : $(NAME)
	mkdir -p logs
	mkdir -p temp

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
	rm -rf ./file_uploads/
	rm -rf ./tmp_upload/

re : fclean all

.PHONY : all clean fclean re