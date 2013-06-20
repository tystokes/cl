#include <utility>
#define __NO_STD_VECTOR // Use cl::vector instead of STL version
#include <CL/cl.hpp>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <ctime>

#define ALWAYS_SHOW_BUILDLOG 1

const std::string hw("Hello World\n");

inline void checkErr(cl_int err, const char * name) {

  if (err != CL_SUCCESS) {
    std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void printTime(clock_t *timer, double *duration) {
  *duration = ( std::clock()/* - *start */) / (double) CLOCKS_PER_SEC;
  std::cout<<"printf: "<< *duration <<'\n';
  //*start = *duration;
}

int main(void) {

  cl_int err;
  
  cl::vector<cl::Platform> platformList;
  cl::Platform::get(&platformList);

  
  checkErr(platformList.size() != 0 ? CL_SUCCESS : -1, "cl::Platform::get");
  std::cerr << platformList.size() << " platform(s)" << std::endl;

  std::string platformVendor;
  std::string platformName;
  platformList[0].getInfo((cl_platform_info)CL_PLATFORM_VENDOR, &platformVendor);
  platformList[0].getInfo((cl_platform_info)CL_PLATFORM_NAME, &platformName);
  
  std::cerr << platformName << " -- " << platformVendor << "\n";
  
  cl_context_properties cprops[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[0])(), 0};
  cl::Context context(CL_DEVICE_TYPE_GPU, cprops, NULL, NULL, &err);
  checkErr(err, "Conext::Context()"); 
  
  /*
  char * outH = new char[hw.length()+1];
  cl::Buffer outCL(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, hw.length()+1, outH, &err);
  checkErr(err, "Buffer::Buffer()");
  */
  
  cl_int input_size;
  std::cout << "enter in size: ";
  std::cin >> input_size;
  cl_int* input = new cl_int[input_size];
  
  for(int i = 0; i < input_size; i++) {
    input[i] = rand() % 10000;
  }
  
  cl_int len[1] = {input_size};
  
  cl::Buffer input_buf(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, input_size * sizeof(cl_int), input, &err);
  cl::Buffer len_buf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), len, &err);
  cl::Buffer dest_buf(context, CL_MEM_WRITE_ONLY, input_size * sizeof(cl_int), NULL, &err);
  
  
  cl::vector<cl::Device> devices;
  devices = context.getInfo<CL_CONTEXT_DEVICES>();
  checkErr(devices.size() > 0 ? CL_SUCCESS : -1, "devices.size() > 0");
  
  for(unsigned int x = 0; x < devices.size(); x++) {
    std::string deviceVendor;
    std::string deviceName;
    err = devices[x].getInfo((cl_device_info)CL_DEVICE_VENDOR, &deviceVendor);
    checkErr(err, "getInfo()"); 
    err = devices[x].getInfo((cl_device_info)CL_DEVICE_NAME, &deviceName);
    checkErr(err, "getInfo()"); 
    std::cerr << "Device " << x << " is: " << deviceName << " -- " << deviceVendor << "\n";
  }
  
  
  std::ifstream file("test.cl");
  checkErr(file.is_open() ? CL_SUCCESS:-1, "test.cl");
  std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
  cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length()));
  
  cl::Program program(context, source);
  
  err = program.build(devices, "");
  
  if (ALWAYS_SHOW_BUILDLOG || err == CL_BUILD_PROGRAM_FAILURE) {
    // Print the log
    std::cout << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
  }
  
  cl::Kernel kernel(program, "mergesort", &err);
  checkErr(err, "Kernel::Kernel()");
  err = kernel.setArg(0, input_buf);
  checkErr(err, "Kernel::setArg(0)");
  err = kernel.setArg(1, dest_buf);
  checkErr(err, "Kernel::setArg(1)");
  err = kernel.setArg(2, len_buf);
  checkErr(err, "Kernel::setArg(2)");
  
  /*
  cl::Kernel kernel(program, "hello", &err);
  checkErr(err, "Kernel::Kernel()");
  err = kernel.setArg(0, outCL);
  checkErr(err, "Kernel::setArg()");
  */
  cl::CommandQueue queue(context, devices[0], 0, &err);
  checkErr(err, "CommandQueue::CommandQueue()");
  
 // program.mergesort(queue, input_size/2, NULL, input_buf, len_buf, dest_buf);
  
  
  cl::Event event;
  err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input_size/2), cl::NullRange, NULL, &event);
  checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");
  event.wait();
  
  /*
  err = queue.enqueueReadBuffer(outCL, CL_TRUE, 0, hw.length()+1, outH);
  checkErr(err, "ComamndQueue::enqueueReadBuffer()");
  
  std::cout << outH;
  */
  cl_int* input_mult = new cl_int[input_size];
  cl_int len_out[1] = {0};
  
  err = queue.enqueueReadBuffer(dest_buf, CL_TRUE, 0, input_size * sizeof(cl_int), input_mult, NULL, &event);
  checkErr(err, "ComamndQueue::enqueueReadBuffer()");
  event.wait();
  err = queue.enqueueReadBuffer(len_buf, CL_TRUE, 0, sizeof(cl_int), len_out, NULL, &event);
  checkErr(err, "ComamndQueue::enqueueReadBuffer()");
  event.wait();
  
  /*err = queue.enqueueCopyBuffer(input_mult, dest_buf, 0, 0, sizeof(cl_int) * input_size, NULL, &event);
  checkErr(err, "ComamndQueue::enqueueCopyBuffer()");
  err = queue.enqueueCopyBuffer(len_out, len_buf, 0, 0, sizeof(cl_int), NULL, &event);
  checkErr(err, "ComamndQueue::enqueueCopyBuffer()");
  */
  /*
  for(int i = 0; i < input_size; i++) {
    std::cout<< input[i] << " ";
  }*/
  
  for(int i = 0; i < input_size; i++) {
    std::cout<< input_mult[i] << " ";
  }
  std::cout << std::endl << len_out[0] << std::endl;
  for(int i = 0; i < input_size - 1; i++) {
    if(input_mult[i] > input_mult[i+1]) {
      std::cout << "implementation bugged: did not sort correctly!" << std::endl;
      std::cout << "elt " << i <<" && " << i+1 << " : " << input_mult[i] << " " << input_mult[i+1] << std::endl; 
      break;
    }
  }
  
  return EXIT_SUCCESS;
}
