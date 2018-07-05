/*
Compile with

clang++ test.cpp OpenCL.lib -std=c++14 -I. -I./CL -m64; ./a

NOTE: need to link to OpenCL.lib as well as -m64 if we're compiling on 64 bit

*/

#include <iostream>
#include <CL/cl.hpp>
#include <omp.h>
#include <vector>

int main(){
    //get all platforms (drivers)
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    // error checking
    if(all_platforms.size() == 0){
        std::cout<<" No platforms found. Check OpenCL installation!\n";
        exit(1);
    }

    // run through all devices and list
    for (auto plat_info : all_platforms){
        std::cout << "Platform: " 
                  << plat_info.getInfo<CL_PLATFORM_NAME>()<<"\n";
        std::cout << "\tDevice list:";
        std::vector<cl::Device> platform_devices;
        plat_info.getDevices(CL_DEVICE_TYPE_ALL, 
                                    &platform_devices);
        for (auto d_info : platform_devices){
            std::cout << "\n\t\t" 
                  <<d_info.getInfo<CL_DEVICE_NAME>() << "\n";
        }
    }

    // OLD CODE
    //get default device of the default platform
    std::vector<cl::Device> all_devices;
    cl::Platform default_platform = all_platforms[0];
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if(all_devices.size() == 0){
        std::cout<<" No devices found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Device default_device = all_devices[0];


    // context is runtime link
    cl::Context context({default_device});
    // program which we want to execute on device
    cl::Program::Sources sources;

    // actual source of the kernel
    // kernel calculates for each element C=A+B
    // As we want that one thread calculates sum of only one element, 
    //    we use get_global_id(0) <-- refers to a 1D array
    //    get_global_id can take 0, 1 or 2 for 1D, 2D or 3D problems
    std::string kernel_code =
    "   void kernel simple_add(global const int* A, "
                              "global const int* B, "
                              "global int* C){      "
    "       C[get_global_id(0)] = A[get_global_id(0)] + B[get_global_id(0)];"
    "}";

    // Add kernel to sources
    sources.push_back({kernel_code.c_str(), kernel_code.length()});
    // build sources and check for build failures @ runtime
    cl::Program program(context,sources);
    if(program.build({default_device})!=CL_SUCCESS){
        std::cout<<" Error building: "
                  << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)
                  << "\n";
        exit(1);
    }

    // example data
    int A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0};

    // create buffers on the device
    cl::Buffer buffer_A(context,CL_MEM_READ_WRITE,sizeof(int)*10);  // TODO: refactor to take input size
    cl::Buffer buffer_B(context,CL_MEM_READ_WRITE,sizeof(int)*10);
    cl::Buffer buffer_C(context,CL_MEM_READ_WRITE,sizeof(int)*10);

    // create queue to which we will push commands for the device.
    // commands pass data from host to device
    cl::CommandQueue queue(context,default_device);

    // make the kernel
    auto simple_add = cl::make_kernel<
            // areguments are 2 input buffers and output buffer
            cl::Buffer&, cl::Buffer&, cl::Buffer&
        >(program, "simple_add");
    cl::EnqueueArgs eargs(queue,
                          cl::NullRange, 
                          cl::NDRange(10), // TODO: refactor. 10 is sizeof(A)
                          cl::NullRange);
    // run the kernel
    simple_add(eargs, buffer_A, buffer_B, buffer_C).wait();

    //read result C from the device to array C
    int C[10]; // result buffer. TODO: refactor
    queue.enqueueReadBuffer(buffer_C, 
                            CL_TRUE, 
                            0, 
                            sizeof(int)*10, 
                            C);

    std::cout<<" result: \n";
    for(int i=0;i<10;i++){
        std::cout<<C[i]<<" ";
    }

    return 0;
}