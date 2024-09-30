CXX=pgc++
CXXFLAGS=-fast -ta=multicore -acc=host -lboost_program_options

NVTXLIB := -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.11/cuda/12.3/include/

all: exe

exe: laplace2d.o jacobi.o
	$(CXX) $(CXXFLAGS) -o $@ $^ ${NVTXLIB}
