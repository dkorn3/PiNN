#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(err, msg) do { \
    if ((err) != CL_SUCCESS) { \
        fprintf(stderr, "%s failed: %d\n", msg, err); \
        return 1; \
    } \
} while (0)

int main(void)
{
    cl_int err;

    const char *source =
        "__kernel void copy_buffer("
        "    __global const uint *input,"
        "    __global uint *output)"
        "{"
        "    size_t i = get_global_id(0);"
        "    output[i] = input[i];"
        "}";

    const cl_uint input[4] = {
        10, 20, 30, 40
    };

    cl_uint output[4] = {0};

    cl_platform_id platform;
    cl_device_id device;

    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    CHECK(err, "clGetDeviceIDs");

    char name[256] = {0};
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), name, NULL);
    printf("OpenCL device: %s\n", name);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK(err, "clCreateContext");

    cl_command_queue queue =
        clCreateCommandQueue(context, device, 0, &err);
    CHECK(err, "clCreateCommandQueue");

    cl_program program =
        clCreateProgramWithSource(context, 1, &source, NULL, &err);
    CHECK(err, "clCreateProgramWithSource");

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    if (err != CL_SUCCESS) {
        char log[4096] = {0};
        clGetProgramBuildInfo(
            program, device, CL_PROGRAM_BUILD_LOG,
            sizeof(log), log, NULL
        );
        fprintf(stderr, "Kernel compilation failed: %d\n%s\n", err, log);
        return 1;
    }

    printf("Kernel compiled successfully.\n");

    cl_mem input_buffer = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(input),
        (void *)input,
        &err
    );
    CHECK(err, "clCreateBuffer input");

    cl_mem output_buffer = clCreateBuffer(
        context,
        CL_MEM_WRITE_ONLY,
        sizeof(output),
        NULL,
        &err
    );
    CHECK(err, "clCreateBuffer output");

    cl_kernel kernel = clCreateKernel(program, "copy_buffer", &err);
    CHECK(err, "clCreateKernel");

    err = clSetKernelArg(kernel, 0, sizeof(input_buffer), &input_buffer);
    CHECK(err, "clSetKernelArg input");

    err = clSetKernelArg(kernel, 1, sizeof(output_buffer), &output_buffer);
    CHECK(err, "clSetKernelArg output");

    size_t global_size = 4;
    size_t local_size = 1;

    printf("Executing kernel...\n");

    fprintf(stderr, "[1] Before kernel enqueue\n");
    err = clEnqueueNDRangeKernel(
        queue, kernel, 1, NULL,
        &global_size, &local_size,
        0, NULL, NULL
    );
    CHECK(err, "clEnqueueNDRangeKernel");
    fprintf(stderr, "[2] Kernel enqueue returned successfully\n");

    fprintf(stderr, "[3] Before clFinish\n");
    err = clFinish(queue);
    CHECK(err, "clFinish");
    fprintf(stderr, "[4] clFinish returned successfully\n");

    fprintf(stderr, "[5] Before readback\n");
    err = clEnqueueReadBuffer(
        queue, output_buffer, CL_TRUE,
        0, sizeof(output), output,
        0, NULL, NULL
    );
    CHECK(err, "clEnqueueReadBuffer");
    fprintf(stderr, "[6] Readback returned successfully\n");

    printf("Input:  ");
    for (int i = 0; i < 4; i++)
        printf("%u ", input[i]);

    printf("\nOutput: ");
    for (int i = 0; i < 4; i++)
        printf("%u ", output[i]);

    printf("\n");

    int passed = 1;
    for (int i = 0; i < 4; i++) {
        if (input[i] != output[i])
            passed = 0;
    }

    clReleaseKernel(kernel);
    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    if (passed)
        printf("SUCCESS: OpenCL kernel produced the correct output!\n");
    else
        printf("FAILURE: Output does not match input.\n");

    return passed ? 0 : 1;
}
