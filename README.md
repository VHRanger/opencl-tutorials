# opencl-tutorials
learning some opencl tooling and compiling

NOTE: the tooling comes from the NVIDIA CUDA toolkit 9.0. You can get the 32 bit openCL.lib but this one includes the 64 bit lib only (which means compiling on -m64)

**Compiling:** clang++ test.cpp -I. -I./CL OpenCL.lib
