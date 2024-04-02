CC = g++
CPPFLAGS = -Wall -O0 -Wno-strict-aliasing -g --std=c++23
ifeq ($(OS),Windows_NT)
	LIBFLAGS = -L./ -lmingw32 -lglew32 -lglfw3 -lopengl32 -lgdi32 -lassimp
	LINKFLAGS =  
else
	LIBFLAGS = -L./ -lGLEW -lglfw -lGL -lX11 -lassimp 
	LINKFLAGS = 
endif

INCLUDE = -Iinclude
ifeq ($(OS),Windows_NT)
	EXEC = scuffed-engine.exe
	RM = del /s /f /q
	RUN = $(EXEC)
else
	EXEC = scuffed-engine
	RM = rm -f
	RUN = ./$(EXEC)
endif

COLOR_RED = [0;31m
COLOR_GREEN = [0;32m
COLOR_YELLOW = [0;33m
COLOR_BLUE = [0;34m
COLOR_MAGENTA = [0;35m
COLOR_CYAN = [0;36m
COLOR_LIGHT_GRAY = [0;37m
COLOR_DARK_GRAY = [1;30m
COLOR_LIGHT_RED = [1;31m
COLOR_LIGHT_GREEN = [1;32m
COLOR_LIGHT_YELLOW = [1;33m
COLOR_LIGHT_BLUE = [1;34m
COLOR_LIGHT_MAGENTA = [1;35m
COLOR_LIGHT_CYAN = [1;36m
COLOR_WHITE = [0;37m
COLOR_BOLD = [1m
COLOR_DIM = [2m
COLOR_UNDERLINED = [4m
COLOR_RESET = [0m

define PRINT_INFO
$(COLOR_BOLD)$(COLOR_LIGHT_BLUE)$(1)$(COLOR_RESET)
endef

define PRINT_SUCCESS
$(COLOR_BOLD)$(COLOR_LIGHT_GREEN)$(1)$(COLOR_RESET)
endef

define PRINT_BUILD
$(COLOR_BOLD)$(COLOR_LIGHT_CYAN)Building$(COLOR_RESET)
endef

define PRINT_LINK
$(COLOR_BOLD)$(COLOR_LIGHT_YELLOW)Linking $(COLOR_RESET)
endef

define PRINT_CLEAN
$(COLOR_BOLD)$(COLOR_LIGHT_RED)Cleaning$(COLOR_RESET)
endef

define PRINT_RUN
$(COLOR_LIGHT_MAGENTA)$(1)$(COLOR_RESET)
endef

define PRINT_PATH
$(foreach file, $(1), $(COLOR_UNDERLINED)$(COLOR_DARK_GRAY)$(file)$(COLOR_RESET))
endef

ifeq ($(OS),Windows_NT)
	WINDOWS_PATH_SUBST = $(subst /,\,$(1))
else
	WINDOWS_PATH_SUBST = $(1)
endif

define TO_WINDOWS_PATH
$(WINDOWS_PATH_SUBST)
endef

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ODIR=obj
IDIR=include
SDIR=src

DEPDIR := .deps
DEPFLAGS_BASE = -MT $@ -MMD -MP -MF $(DEPDIR)
DEPFLAGS = $(DEPFLAGS_BASE)/$*.d
DEPFLAGSMAIN = $(DEPFLAGS_BASE)/main.d

SOURCES := $(call rwildcard,$(SDIR),*.cpp)
OBJ := $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)
OBJ += $(ODIR)/main.o

ifeq ($(OS),Windows_NT)
	ECHO_COMMAND_QUOTATION_MARK =
else
	ECHO_COMMAND_QUOTATION_MARK = "
endif

define ECHO
	echo $(ECHO_COMMAND_QUOTATION_MARK)$(1)$(ECHO_COMMAND_QUOTATION_MARK)
endef

default: 
	@$(call ECHO,$(call PRINT_SUCCESS,Starting build...))
	@$(call ECHO,$(call PRINT_INFO,Building) 	$(call PRINT_PATH,$(EXEC)))
	@$(call ECHO,$(call PRINT_INFO,Sources) 	$(call PRINT_PATH,$(SOURCES)))
	@$(call ECHO,$(call PRINT_INFO,Objects) 	$(call PRINT_PATH,$(OBJ)))
	@$(call ECHO,$(call PRINT_INFO,Dependencies) 	$(call PRINT_PATH,$(DEPFILES)))
	@$(MAKE) $(EXEC) -j8 -s
	@$(call ECHO,$(call PRINT_SUCCESS,Build successful!))


obj/main.o : main.cpp
obj/main.o : main.cpp 
	@$(call ECHO,$(call PRINT_BUILD)	$(call PRINT_PATH,$@))
	$(CC) -c $(DEPFLAGS_BASE) $(DEPFLAGSMAIN) $(CPPFLAGS) -Iinclude -Wno-delete-non-virtual-dtor $(LIBFLAGS) $(INCLUDE) $< -o $@

obj/%.o : src/%.cpp
obj/%.o : src/%.cpp
	@$(call ECHO,$(call PRINT_BUILD)	$(call PRINT_PATH,$@))
	$(CC) -c $(DEPFLAGS_BASE) $(DEPFLAGS) $(CPPFLAGS) $(LIBFLAGS) $(INCLUDE) -Iinclude $< -o $@ 

run:
	$(RUN)

debug:
	gdb $(EXEC)

$(EXEC): $(OBJ)
	@$(call ECHO,$(call PRINT_LINK)	$(call PRINT_PATH,$@))
	$(CC) -o $@ $^ $(CPPFLAGS) $(LIBFLAGS) $(LINKFLAGS)

$(DEPDIR): ; @mkdir $@

DEPFILES := $(SOURCES:$(SDIR)/%.cpp=$(DEPDIR)/%.d)
DEPFILES += $(DEPDIR)/main.d
# $(info $(DEPFILES))
$(DEPFILES):


include $(wildcard $(DEPFILES))

ifeq ($(OS),Windows_NT)
	IF_EXISTS = if exist
	ENF_IF_EXISTS =
	NULL = nul
else
	IF_EXISTS = test -f
	ENF_IF_EXISTS = &&
	NULL = /dev/null
endif

define REMOVE_FILE
	@$(IF_EXISTS) $(1) $(ENF_IF_EXISTS) $(call ECHO,$(call PRINT_CLEAN)	$(call PRINT_PATH, $(call TO_WINDOWS_PATH,$(1))))

	@$(IF_EXISTS) $(1) $(ENF_IF_EXISTS) $(RM) $(call TO_WINDOWS_PATH,$(1)) > $(NULL)

endef


define REMOVE
	$(foreach file, $(1), $(foreach f, $(wildcard $(file)), $(call REMOVE_FILE,$(f))))
endef



clean:
ifeq ($(OS),Windows_NT)
	$(call REMOVE, $(EXEC) $(ODIR)/*.o $(DEPDIR)/*.d)
else
	$(call REMOVE, $(EXEC) $(ODIR)/*.o $(DEPDIR)/*.d)
endif

reinstall: clean default

relink: 
ifeq ($(OS),Windows_NT)
	$(call REMOVE,$(EXEC))
else
	$(call REMOVE,$(EXEC))
endif
	@$(MAKE) $(EXEC) -j8 -s

.PHONY: clean run debug reinstall