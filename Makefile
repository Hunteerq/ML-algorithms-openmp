


CC := g++-10


ARCH := core2 # Replace this your CPU architecture.
# core2 is pretty safe for most modern machines.

CFLAGS := -O3 -fopenmp -m64 -std=c++11

COMPILE_COMMAND := $(CC) $(CFLAGS)

CC_MPI := mpic++

CFLAGS_MPI := -std=c++11 -lstdc++ -lm

COMPILE_COMMAND_MPI := $(CC_MPI) $(CFLAGS_MPI)

COMPILE_COMMAND_HYBRID := $(CC_MPI) -qopenmp $(CFLAGS_MPI)

OUTPUT := labels_encoder_omp labels_encoder_mpi labels_encoder_hybrid

labels_encoder_omp: labels_encoder_omp.cpp
	$(COMPILE_COMMAND) -o labels_encoder_omp labels_encoder_omp.cpp

labels_encoder_mpi: labels_encoder_mpi.cpp
	$(COMPILE_COMMAND_MPI) -o labels_encoder_mpi labels_encoder_mpi.cpp

labels_encoder_hybrid: labels_encoder_hybrid.cpp
	$COMPILE_COMMAND_HYBRID) -o labels_encoder_hybrid labels_encoder_hybrid.cpp

#mpicc  -o mx min_max.cpp -lstdc++ -lm

#
#knn: knn.cpp
#	$(COMPILE_COMMAND) -o knn knn.cpp

## --bind-to none w mpirun

clean:
	rm -f *.o $(OUTPUT)
