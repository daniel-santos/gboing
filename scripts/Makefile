TARGET        = qsort
SRC_DIR       = $(GBOING_DIR)/src/test
INCLUDE_DIR   = $(GBOING_DIR)/include
CPPFLAGS     += -I$(INCLUDE_DIR)

CFLAGS_no_flto = $(filter-out -flto,$(CFLAGS))

STRIP         = $(BINUTILS_PREFIX)strip

_HEADERS = gboing/compiler-gcc.h gboing/compiler.h gboing/cpp.h gboing/qsort-template.h
HEADERS = $(patsubst %,$(INCLUDE_DIR)/%,$(_HEADERS))
OBJECTS = qsort.o glibc-qsort.o

# Special rules for object files we don't want built with -flto
qsort-instantiate.o: $(SRC_DIR)/qsort-instantiate.c $(HEADERS)
	$(CC) $(CFLAGS_no_flto) $(CPPFLAGS) -c $< -o $@
#	$(STRIP) -s $@

glibc-qsort.o: $(SRC_DIR)/glibc-qsort.c $(HEADERS)
	$(CC) $(CFLAGS_no_flto) $(CPPFLAGS) -c $< -o $@

# Default .o recipe
%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJECTS) -o $@

.PHONY: clean all

clean:
	rm -f $(TARGET) *.o

all: $(TARGET) qsort-instantiate.o

