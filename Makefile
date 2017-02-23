.DEFAULT_GOAL := all
.PHONY := all build rebuild clean debug release
OS_NAME := $(shell uname)

PROG_NAME = animation
SRC_DIR = src
RES_DIR = res
SHADER_DIR = $(RES_DIR)/shaders

CXX = g++
CXX_BASE_FLAGS = -std=c++14 -MD -MP
CXX_FLAGS = $(CXX_BASE_FLAGS) -Wall -Wconversion $(BUILD_MODE_FLAGS) $(LIB_DEFINES)

ifeq ($(OS_NAME), Linux)
OS_LINKER_FLAGS =
endif
ifeq ($(OS_NAME), Darwin)
OS_LINKER_FLAGS = -framework OpenGL
endif

LIB_DEFINES = $(GLM_DEFINES)

# ------------------------------------------------------------------------------
# External Sources
# ------------------------------------------------------------------------------
EXTERNAL_CPP_FILES = stb_image.cpp
PREFIX_EXTERNAL_CPP_FILES = $(addprefix ext/,$(EXTERNAL_CPP_FILES))

# ------------------------------------------------------------------------------
# Renderer Sources
# ------------------------------------------------------------------------------

RENDERER_CPP_FILES = Pass.cpp Shader.cpp Texture.cpp UniformBuffer.cpp
PREFIX_RENDERER_CPP_FILES = $(addprefix Renderer/,$(RENDERER_CPP_FILES))

# ------------------------------------------------------------------------------
# Scene Sources
# ------------------------------------------------------------------------------

SCENE_MODEL_CPP_FILES = Skeleton.cpp Skin.cpp Morph.cpp parsing.cpp \
Animation.cpp Cloth.cpp
PREFIX_SCENE_MODEL_CPP_FILES = $(addprefix Model/,$(SCENE_MODEL_CPP_FILES))

SCENE_CPP_FILES = Camera.cpp Graph.cpp Object.cpp Skybox.cpp Model.cpp
PREFIX_SCENE_CPP_FILES = $(addprefix Scene/,$(SCENE_CPP_FILES) \
$(PREFIX_SCENE_MODEL_CPP_FILES)

# ------------------------------------------------------------------------------
# Root Sources
# ------------------------------------------------------------------------------

CPP_FILES = CommandLine.cpp DOFWindow.cpp main.cpp Program.cpp Renderer.cpp \
	    Scene.cpp Timer.cpp Window.cpp
PREFIX_CPP_FILES = $(addprefix src/$(CPP_FILES) $(PREFIX_SCENE_CPP_FILES) \
$(PREFIX_RENDERER_CPP_FILES) $(PREFIX_EXTERNAL_CPP_FILES))

# ------------------------------------------------------------------------------
# Unprefixed Sources
# ------------------------------------------------------------------------------

UNPREFIX_CPP_FILES = $(RENDERER_CPP_FILES) $(SCENE_CPP_FILES) $(CPP_FILES) \
$(EXTERNAL_CPP_FILES) $(SCENE_MODEL_CPP_FILES)



OBJ_FILES = $(UNPREFIX_CPP_FILES:%.cpp=%.o)
PREFIX_OBJ_FILES = $(addprefix build/,$(OBJ_FILES))

DEP_FILES = $(PREFIX_OBJ_FILES:%.o=%.d)

PKG_CONFIG_LIBS = glfw3 glew
MANUAL_LIBS = $(shell fltk-config --ldflags)
LIBS = $(MANUAL_LIBS) $(shell pkg-config --libs $(PKG_CONFIG_LIBS))

PKG_CONFIG_INCLUDE = glfw3 glew
MANUAL_INCLUDE = $(shell fltk-config --cflags)
INCLUDE = $(MANUAL_INCLUDE) $(shell pkg-config --cflags $(PKG_CONFIG_INCLUDE))

# ----------------------------------------------------------
# --- Targets ----------------------------------------------
# ----------------------------------------------------------

all : build

debug : clean
	$(call padEcho,Reconfiguring for debug mode...)
	$(file >buildMode,BUILD_MODE_FLAGS = -g -Og)
	$(file >>buildMode,BUILD_MODE = Debug)

release : clean
	  $(call padEcho,Reconfiguring for release mode...)
	  $(file >buildMode,BUILD_MODE_FLAGS = -DNDEBUG -Ofast)
	  $(file >>buildMode,BUILD_MODE = Release)

build : $(PREFIX_OBJ_FILES)
	$(call padEcho,linking $(PROG_NAME) in $(BUILD_MODE) mode...)
	$(CXX) -o $(PROG_NAME) $(PREFIX_OBJ_FILES) $(CXX_FLAGS) $(INCLUDE) \
$(LIBS) $(OS_LINKER_FLAGS)
	$(call padEcho,done!)

build/stb_image.o : src/ext/stb_image.cpp
		    $(call compileWithOptions,$<,$@,$(CXX_BASE_FLAGS))

build/%.o : src/%.cpp
	  $(call compile,$<,$@)

build/%.o : src/Renderer/%.cpp
	  $(call compile,$<,$@)

build/%.o : src/Scene/%.cpp
	  $(call compile,$<,$@)

build/%.o : src/Scene/Model/%.cpp
	  $(call compile,$<,$@)

rebuild : clean build

clean :
	$(call padEcho,deleting temporary files...)
	$(RM) $(PREFIX_OBJ_FILES)
	$(RM) $(DEP_FILES)
	$(RM) core
	$(RM) *~
	$(RM) $(PROG_NAME)
	$(RM) $(SRC_DIR)/*~
	$(RM) $(SRC_DIR)/Renderer/*~
	$(RM) $(SRC_DIR)/Scene/*~
	$(RM) $(SRC_DIR)/Scene/Model/*~
	$(RM) $(RES_DIR)/*~
	$(RM) $(SHADER_DIR)/*~

# ----------------------------------------------------------
# --- Functions --------------------------------------------
# ----------------------------------------------------------

define compile
$(call compileWithOptions,$(1),$(2),$(CXX_FLAGS))
endef

define compileWithOptions
$(call padEcho,compiling $(1)...)
$(CXX) -c $(1) $(3) $(INCLUDE) $(LIBS) -o $(2)
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

-include $(DEP_FILES)

# ----------------------------------------------------------
# --- Include configuration files --------------------------
# ----------------------------------------------------------

-include buildMode
-include glmDefines
