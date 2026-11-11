#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(call) do { \
    cl_int e = (call); \
    if (e != CL_SUCCESS) { \
        fprintf(stderr, "%s failed: %d\n", #call, e); \
        return 1; \
    } \
} while (0)

int main(void)
{
    cl_int err;
    cl_uint count = 0;

    CHECK(clGetPlatformIDs(0, NULL, &count));
    if (!count) {
        fprintf(stderr, "No OpenCL platforms found\n");
        return 1;
    }

    cl_platform_id platform;
    CHECK(clGetPlatformIDs(1, &platform, NULL));

    cl_device_id device;
    CHECK(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL));

    char name[256] = {0};
    CHECK(clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), name, NULL));
    printf("OpenCL device: %s\n", name);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (!context || err != CL_SUCCESS) {
        fprintf(stderr, "clCreateContext failed: %d\n", err);
        return 1;
    }

    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    if (!queue || err != CL_SUCCESS) {
        fprintf(stderr, "clCreateCommandQueue failed: %d\n", err);
        clReleaseContext(context);
        return 1;
    }

    const size_t size = 4096;
    unsigned char input[4096], output[4096];

    for (size_t i = 0; i < size; ++i)
        input[i] = (unsigned char)(i * 37u + 11u);

    memset(output, 0, sizeof(output));

    printf("Creating device buffer...\n");
    cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &err);
    if (!buffer || err != CL_SUCCESS) {
        fprintf(stderr, "clCreateBuffer failed: %d\n", err);
        return 1;
    }

    printf("Writing buffer...\n");
    CHECK(clEnqueueWriteBuffer(queue, buffer, CL_TRUE, 0, size, input, 0, NULL, NULL));

    printf("Reading buffer...\n");
    CHECK(clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, size, output, 0, NULL, NULL));

    if (memcmp(input, output, size) != 0) {
        fprintf(stderr, "Buffer data mismatch\n");
        return 1;
    }

    printf("OpenCL buffer write/read: PASSED\n");

    clReleaseMemObject(buffer);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    puts("OpenCL memory test completed.");
    return 0;
}
