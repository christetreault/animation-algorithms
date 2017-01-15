.DEFAULT_GOAL := all
.PHONY := all build rebuild clean debug release
OS_NAME := $(shell uname)

PROG_NAME = skeleton
SRC_DIR = src
RES_DIR = res
SHADER_DIR = $(RES_DIR)/shaders

CXX = g++
CXX_FLAGS = -g -Wall -std=c++14 -Ofast -MD -MP \
$(BUILD_MODE_FLAGS) $(LIB_DEFINES)

ifeq ($(OS_NAME), Linux)
OS_LINKER_FLAGS =
endif
ifeq ($(OS_NAME), Darwin)
OS_LINKER_FLAGS = -framework OpenGL
endif

LIB_DEFINES = $(GLM_DEFINES)

OBJ_FILES = main.o Window.o Renderer.o Shader.o Program.o Timer.o Object.o \
Scene.o UniformBuffer.o Graph.o Pass.o Texture.o Camera.o Skybox.o Skeleton.o

PKG_CONFIG_LIBS = glfw3 glew assimp
MANUAL_LIBS =
LIBS = $(MANUAL_LIBS) $(shell pkg-config --libs $(PKG_CONFIG_LIBS))

PKG_CONFIG_INCLUDE = glfw3 glew assimp
MANUAL_INCLUDE =
INCLUDE = $(MANUAL_INCLUDE) $(shell pkg-config --cflags $(PKG_CONFIG_INCLUDE))

# ----------------------------------------------------------
# --- Targets ----------------------------------------------
# ----------------------------------------------------------

all : build

debug : clean
	$(call padEcho,Reconfiguring for debug mode...)
	$(file >buildMode,BUILD_MODE_FLAGS =)
	$(file >>buildMode,BUILD_MODE = Debug)

release : clean
	  $(call padEcho,Reconfiguring for release mode...)
	  $(file >buildMode,BUILD_MODE_FLAGS = -DNDEBUG)
	  $(file >>buildMode,BUILD_MODE = Release)

build : $(OBJ_FILES)
	$(call padEcho,linking $(PROG_NAME) in $(BUILD_MODE) mode...)
	$(CXX) -o $(PROG_NAME) $(OBJ_FILES) $(CXX_FLAGS) $(INCLUDE) $(LIBS) $(OS_LINKER_FLAGS)
	$(call padEcho,done!)

main.o : $(SRC_DIR)/main.cpp
	 $(call compile,main.cpp)

Window.o : $(SRC_DIR)/Window.cpp $(SRC_DIR)/Window.hpp
	   $(call compile,Window.cpp)

Renderer.o : $(SRC_DIR)/Renderer.cpp $(SRC_DIR)/Renderer.hpp
	     $(call compile,Renderer.cpp)

Shader.o : $(SRC_DIR)/Renderer/Shader.cpp $(SRC_DIR)/Renderer/Shader.hpp
	   $(call compile,Renderer/Shader.cpp)

Program.o : $(SRC_DIR)/Program.cpp $(SRC_DIR)/Program.hpp
	    $(call compile,Program.cpp)

Timer.o : $(SRC_DIR)/Timer.cpp $(SRC_DIR)/Timer.hpp
	  $(call compile,Timer.cpp)

Object.o : $(SRC_DIR)/Scene/Object.cpp $(SRC_DIR)/Scene/Object.hpp
	   $(call compile,Scene/Object.cpp)

Scene.o : $(SRC_DIR)/Scene.cpp $(SRC_DIR)/Scene.hpp
	  $(call compile,Scene.cpp)

UniformBuffer.o : $(SRC_DIR)/Renderer/UniformBuffer.cpp \
		  $(SRC_DIR)/Renderer/UniformBuffer.hpp
		  $(call compile,Renderer/UniformBuffer.cpp)

Graph.o : $(SRC_DIR)/Scene/Graph.cpp $(SRC_DIR)/Scene/Graph.hpp
	  $(call compile,Scene/Graph.cpp)

Pass.o : $(SRC_DIR)/Renderer/Pass.cpp $(SRC_DIR)/Renderer/Pass.hpp
	 $(call compile,Renderer/Pass.cpp)

Texture.o : $(SRC_DIR)/Renderer/Texture.cpp $(SRC_DIR)/Renderer/Texture.hpp
	    $(call compile,Renderer/Texture.cpp)

Camera.o : $(SRC_DIR)/Scene/Camera.cpp $(SRC_DIR)/Scene/Camera.hpp
	   $(call compile,Scene/Camera.cpp)

Skybox.o : $(SRC_DIR)/Scene/Skybox.cpp $(SRC_DIR)/Scene/Skybox.hpp
	   $(call compile,Scene/Skybox.cpp)

Skeleton.o : $(SRC_DIR)/Scene/Skeleton.cpp $(SRC_DIR)/Scene/Skeleton.hpp
	     $(call compile,Scene/Skeleton.cpp)

rebuild : clean build

clean :
	$(call padEcho,deleting temporary files...)
	$(RM) *.o
	$(RM) *.d
	$(RM) core
	$(RM) *~
	$(RM) $(PROG_NAME)
	$(RM) $(SRC_DIR)/*~
	$(RM) $(SRC_DIR)/Renderer/*~
	$(RM) $(SRC_DIR)/Scene/*~
	$(RM) $(RES_DIR)/*~
	$(RM) $(SHADER_DIR)/*~

# ----------------------------------------------------------
# --- Functions --------------------------------------------
# ----------------------------------------------------------

define compile
$(call padEcho,compiling $(1)...)
$(CXX) -c $(SRC_DIR)/$(1) $(CXX_FLAGS) $(INCLUDE) $(LIBS)
endef

define padEcho
@echo
@echo --------------------------------------------------------------------------------
@echo $(1)
@echo --------------------------------------------------------------------------------
@echo
endef

# ----------------------------------------------------------
# --- Include dependency files -----------------------------
# ----------------------------------------------------------

-include $(OBJ_FILES:%.o=%.d)

# ----------------------------------------------------------
# --- Include configuration files --------------------------
# ----------------------------------------------------------

-include buildMode
-include glmDefines
