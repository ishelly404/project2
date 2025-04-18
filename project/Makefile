################################################################################
#include <string.h>
# Variables common to all programs.
CC = g++ -D_GLIBCXX_USE_CXX11_ABI=0
MPICC = mpic++ -D_GLIBCXX_USE_CXX11_ABI=0 -DOMPI_SKIP_MPICXX=1

# When running locally, add the flag -no-pie
# ref: https://www.redhat.com/en/blog/position-independent-executables-pie
FLAGS = -Wextra -Wall -Iinclude -no-pie -g $(shell pkg-config --cflags libpng)

LIBS = raytrace
LIBSPATH = objs/x86_64
LIBSPATH := $(addprefix -L,$(LIBSPATH))
LIBS := $(addprefix -l,$(LIBS))
LIBS_PNG := $(shell pkg-config --libs libpng)

################################################################################
# Variables used by sequential code.
SEQ_BIN = raytrace_seq
SEQ_SRC = main_seq.cpp

SEQ_SRC := $(addprefix src/,$(SEQ_SRC))
################################################################################
# Variables used by MPI code.
MPI_BIN = raytrace_mpi
MPI_SRC = master.cpp main_mpi.cpp slave.cpp

MPI_SRC := $(addprefix src/,$(MPI_SRC))
################################################################################
# Variables used by MPI code.
PNG_BIN = png_compare
PNG_SRC = image_operations.cpp

PNG_SRC := $(addprefix src/tools/,$(PNG_SRC))
################################################################################
all:  $(SEQ_BIN) $(MPI_BIN) $(PNG_BIN)

$(SEQ_BIN): $(SEQ_SRC)
	$(CC) $(SEQ_SRC) $(FLAGS) $(LIBS) $(LIBSPATH) $(LIBS_PNG) -o $(SEQ_BIN)

$(MPI_BIN): $(MPI_SRC)
	$(MPICC) $(MPI_SRC) $(FLAGS) $(LIBS) $(LIBSPATH) $(LIBS_PNG) -o $(MPI_BIN)

$(PNG_BIN): $(PNG_SRC)
	$(CC) $(PNG_SRC) $(FLAGS) $(LIBS_PNG) -o $(PNG_BIN)

clean:
	rm -f $(SEQ_BIN) $(MPI_BIN) $(PNG_BIN)
# Comment out if you would like logs to persist through makes
	rm -f -d -r std 
# Comment out if you would like renders to persist through makes
	rm -f -d -r renders 
