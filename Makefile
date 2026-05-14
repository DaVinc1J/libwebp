CC = clang
CXX = clang++

CFLAGS = -std=c11 \
				 -g \
				 -fsanitize=address \
				 -I/opt/homebrew/Cellar/glfw/3.4/include \
				 -I/opt/homebrew/Cellar/cglm/0.9.6/include \
				 -I/Users/tristanlowe/VulkanSDK/1.4.313.1/macOS/include \
				 -I/opt/homebrew/Cellar/freetype/2.14.1_1/include/freetype2/

CXXFLAGS = -std=c++17 \
					 -g \
					 -fsanitize=address \
					 -I/opt/homebrew/Cellar/glfw/3.4/include \
					 -I/opt/homebrew/Cellar/cglm/0.9.6/include \
					 -I/Users/tristanlowe/VulkanSDK/1.4.313.1/macOS/include \
					 -I/opt/homebrew/Cellar/freetype/2.14.1_1/include/freetype2 \
					 -Wno-nullability-completeness

LDFLAGS = \
					-fsanitize=address \
					-L/opt/homebrew/Cellar/glfw/3.4/lib -lglfw \
					-L/Users/tristanlowe/VulkanSDK/1.4.313.1/macOS/lib -lvulkan \
					-Wl,-rpath,/Users/tristanlowe/VulkanSDK/1.4.313.1/macOS/lib \
					-framework Metal \
					-framework QuartzCore \
					-framework Cocoa \
					-lz

SRC_C = $(wildcard src/*.c)
OBJ_C = $(SRC_C:src/%.c=build/%.o)

SRC_M = $(wildcard src/*.m)
OBJ_M = $(SRC_M:src/%.m=build/%.o)

LBR_OBJ = build/libraries.o

OBJ = $(OBJ_C) $(OBJ_M) $(LBR_OBJ)

BIN = build/app

#########################
### Shader Compile ###
#########################

VERT = src/shaders/fullscreen.vert
FRAG = src/shaders/fullscreen.frag

VERT_SPV = src/shaders/fullscreen.vert.spv
FRAG_SPV = src/shaders/fullscreen.frag.spv

#########################
### Default Build All ###
#########################

.PHONY: all
all: shaders $(BIN)

$(BIN): $(OBJ)
	mkdir -p build
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.m
	mkdir -p build
	$(CC) $(CFLAGS) -fobjc-arc -c $< -o $@

##########################
### Fast Relink Only ###
##########################

.PHONY: fast
fast: $(BIN)

#########################
### VMA Wrapper Only ###
#########################

.PHONY: lbr
lbr: $(LBR_OBJ)

$(LBR_OBJ): src/libraries/libraries.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

#######################
### Shader Targets ###
#######################

.PHONY: shaders
shaders: $(VERT_SPV) $(FRAG_SPV)

$(VERT_SPV): $(VERT)
	glslangValidator -V $< -o $@

$(FRAG_SPV): $(FRAG)
	glslangValidator -V $< -o $@

#######################
### Clean Targets ###
#######################

.PHONY: clean
clean:
	rm -rf build
	rm -f $(VERT_SPV) $(FRAG_SPV)

.PHONY: clean-vma
clean-vma:
	rm -f $(LBR_OBJ)

.PHONY: clean-shaders
clean-shaders:
	rm -f $(VERT_SPV) $(FRAG_SPV)
