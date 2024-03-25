CC = g++
CPPFLAGS = -Wall -Ofast -Wno-strict-aliasing -g --std=c++23
ifeq ($(OS),Windows_NT)
	LIBFLAGS = -L./ -lmingw32 -lglew32 -lglfw3 -lopengl32 -lgdi32 -lassimp
	LINKFLAGS =  
else
	LIBFLAGS = -L./ -lGLEW -lglfw -lGL -lX11 -lassimp
	LINKFLAGS = 
endif

INCLUDE = -Iinclude
ifeq ($(OS),Windows_NT)
	EXEC = ScuffedEngine.exe
	RM = del /s /f /q
	RUN = $(EXEC)
else
	EXEC = ScuffedEngine
	RM = rm -f
	RUN = ./$(EXEC)
endif

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

default: 
	@echo Building $(EXEC)
	@echo Sources: $(SOURCES)
	@echo Objects: $(OBJ)
	@echo Dependencies: $(DEPFILES)
	@$(MAKE) $(EXEC) -j8 -s

obj/main.o : main.cpp
obj/main.o : main.cpp 
	@echo Building main.o
	$(CC) -c $(DEPFLAGS_BASE) $(DEPFLAGSMAIN) $(CPPFLAGS) -Iinclude -Wno-delete-non-virtual-dtor $(LIBFLAGS) $(INCLUDE) $< -o $@

obj/%.o : src/%.cpp
obj/%.o : src/%.cpp
	@echo Building $@
	$(CC) -c $(DEPFLAGS_BASE) $(DEPFLAGS) $(CPPFLAGS) $(LIBFLAGS) $(INCLUDE) -Iinclude $< -o $@ 

run:
	$(RUN)

debug:
	gdb $(EXEC)

$(EXEC): $(OBJ)
	@echo "Linking $(EXEC)"
	$(CC) -o $@ $^ $(CPPFLAGS) $(LIBFLAGS) $(LINKFLAGS)

$(DEPDIR): ; @mkdir $@

DEPFILES := $(SOURCES:$(SDIR)/%.cpp=$(DEPDIR)/%.d)
DEPFILES += $(DEPDIR)/main.d
# $(info $(DEPFILES))
$(DEPFILES):


include $(wildcard $(DEPFILES))

clean:
ifeq ($(OS),Windows_NT)
	$(RM) $(EXEC) $(ODIR)\*.o $(DEPDIR)\*.d
else
	$(RM) $(EXEC) $(ODIR)/*.o $(DEPDIR)/*.d
endif

reinstall: clean default