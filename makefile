CC = gcc
CFLAGS = -Wall -Wextra -lm -ggdb -Isrc
DEPDIR = .d
DEPFLAGS = -MT $@ -MMD -MF $(DEPDIR)/$*.d
SOURCEDIR = src
OBJECTDIR = obj
SOURCES = $(wildcard $(SOURCEDIR)/*.c) $(wildcard $(SOURCEDIR)/**/*.c)
HEADERS = $(wildcard $(SOURCEDIR)/*.h) $(wildcard $(SOURCEDIR)/**/*.h)
OBJECTS = $(patsubst $(SOURCEDIR)/%.c, $(OBJECTDIR)/%.o, $(SOURCES))
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

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.c $(DEPDIR)/%.d
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

$(TESTOBJDIR)/%.o: $(TESTDIR)/%.c $(DEPDIR)/%.d
	@mkdir -p $(@D)
	@mkdir -p $(patsubst $(TESTOBJDIR)%, $(DEPDIR)%, $(@D))
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(DEPDIR)/%.d: ;

-include $(DEPFILES)
