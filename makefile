CC = g++
CFLAGS = -Wall -Wextra -lm -lcurses -ggdb -Isrc
DEPDIR = .d
DEPFLAGS = -MT $@ -MMD -MF $(DEPDIR)/$*.d
SOURCEDIR = src
OBJECTDIR = obj
SOURCES = $(wildcard $(SOURCEDIR)/*.cpp) $(wildcard $(SOURCEDIR)/**/*.cpp)
HEADERS = $(wildcard $(SOURCEDIR)/*.h) $(wildcard $(SOURCEDIR)/**/*.h)
OBJECTS = $(patsubst $(SOURCEDIR)/%.cpp, $(OBJECTDIR)/%.o, $(SOURCES))

DEPFILES = $(patsubst $(SOURCEDIR)/%.cpp, $(DEPDIR)/%.d, $(SOURCES))
TARGET = outfile

#Test variables
TESTDIR = tests
TESTOBJDIR = tests/obj
TEST_SOURCES = $(wildcard $(TESTDIR)/*.cpp) $(wildcard $(TESTDIR)/**/*.cpp)
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.cpp, $(TESTOBJDIR)/%.o, $(TEST_SOURCES))
TEST_TARGET = testout

.PHONY: clean all run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) -std=c++14

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.cpp $(DEPDIR)/%.d
	@mkdir -p $(@D)
	@mkdir -p $(patsubst $(OBJECTDIR)%, $(DEPDIR)%, $(@D))
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTDIR) $(TARGET) $(DEPDIR) $(TESTOBJDIR)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(filter-out $(OBJECTDIR)/main.o, $(OBJECTS))
	$(CC) -o $@ $^ $(CFLAGS)

$(TESTOBJDIR)/%.o: $(TESTDIR)/%.cpp $(DEPDIR)/%.d
	@mkdir -p $(@D)
	@mkdir -p $(patsubst $(TESTOBJDIR)%, $(DEPDIR)%, $(@D))
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(DEPDIR)/%.d: ;

-include $(DEPFILES)
