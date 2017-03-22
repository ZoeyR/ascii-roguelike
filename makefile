CC = gcc
CPP = g++
CFLAGS = -Wall -Wextra -lm -lcurses -ggdb -Isrc
DEPDIR = .d
DEPFLAGS = -MT $@ -MMD -MF $(DEPDIR)/$*.d
SOURCEDIR = src
OBJECTDIR = obj
SOURCES = $(wildcard $(SOURCEDIR)/*.c) $(wildcard $(SOURCEDIR)/**/*.c)
HEADERS = $(wildcard $(SOURCEDIR)/*.h) $(wildcard $(SOURCEDIR)/**/*.h)
OBJECTS = $(patsubst $(SOURCEDIR)/%.c, $(OBJECTDIR)/%.o, $(SOURCES))

CPP_SOURCES = $(wildcard $(SOURCEDIR)/*.cpp) $(wildcard $(SOURCEDIR)/**/*.cpp)
CPP_OBJECTS = $(patsubst $(SOURCEDIR)/%.cpp, $(OBJECTDIR)/%.opp, $(CPP_SOURCES))

DEPFILES = $(patsubst $(SOURCEDIR)/%.c, $(DEPDIR)/%.d, $(SOURCES))
TARGET = outfile

#Test variables
TESTDIR = tests
TESTOBJDIR = tests/obj
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c) $(wildcard $(TESTDIR)/**/*.c)
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.c, $(TESTOBJDIR)/%.o, $(TEST_SOURCES))
TEST_TARGET = testout

.PHONY: clean all run test

all: $(TARGET)

$(TARGET): $(OBJECTS) $(CPP_OBJECTS)
	$(CPP) -o $@ $^ $(CFLAGS) -std=c++14

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.c $(DEPDIR)/%.d
	@mkdir -p $(@D)
	@mkdir -p $(patsubst $(OBJECTDIR)%, $(DEPDIR)%, $(@D))
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJECTDIR)/%.opp: $(SOURCEDIR)/%.cpp $(DEPDIR)/%.d
	@mkdir -p $(@D)
	@mkdir -p $(patsubst $(OBJECTDIR)%, $(DEPDIR)%, $(@D))
	$(CPP) $(DEPFLAGS) $(CFLAGS) -std=c++14 -c $< -o $@

clean:
	rm -rf $(OBJECTDIR) $(TARGET) $(DEPDIR) $(TESTOBJDIR)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(filter-out $(OBJECTDIR)/main.o, $(OBJECTS))
	$(CC) -o $@ $^ $(CFLAGS)

$(TESTOBJDIR)/%.o: $(TESTDIR)/%.c $(DEPDIR)/%.d
	@mkdir -p $(@D)
	@mkdir -p $(patsubst $(TESTOBJDIR)%, $(DEPDIR)%, $(@D))
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(DEPDIR)/%.d: ;

-include $(DEPFILES)
