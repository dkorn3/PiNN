#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

#define VC_SM_CMA_MAGIC_TYPE 'J'
#define VC_SM_CMA_CMD_ALLOC  0x5A
#define VC_SM_CMA_RESOURCE_NAME 32

struct vc_sm_cma_ioctl_alloc {
    uint32_t size;
    uint32_t num;
    uint32_t cached;
    uint32_t pad;
    uint8_t name[VC_SM_CMA_RESOURCE_NAME];
    int32_t handle;
    uint32_t vc_handle;
    uint64_t dma_addr;
};

#define VC_SM_CMA_IOCTL_MEM_ALLOC \
    _IOR(VC_SM_CMA_MAGIC_TYPE, VC_SM_CMA_CMD_ALLOC, \
         struct vc_sm_cma_ioctl_alloc)

int main(void)
{
    int dev = open("/dev/vcsm-cma", O_RDWR);
    if (dev < 0) {
        perror("open /dev/vcsm-cma");
        return 1;
    }

    struct vc_sm_cma_ioctl_alloc alloc = {0};
    alloc.size = 4096;
    alloc.num = 1;
    alloc.cached = 0;
    snprintf((char *)alloc.name, sizeof(alloc.name), "PiNN-probe");

    printf("Requesting 4096-byte CMA allocation...\n");

    if (ioctl(dev, VC_SM_CMA_IOCTL_MEM_ALLOC, &alloc) < 0) {
        perror("CMA allocation ioctl");
        close(dev);
        return 1;
    }

    printf("DMA-BUF fd: %d\n", alloc.handle);
    printf("VideoCore handle: 0x%08x\n", alloc.vc_handle);
    printf("DMA address: 0x%llx\n",
           (unsigned long long)alloc.dma_addr);

    if (alloc.handle < 0) {
        fprintf(stderr, "Invalid DMA-BUF fd\n");
        close(dev);
        return 1;
    }

    void *ptr = mmap(NULL, alloc.size,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED, alloc.handle, 0);

    if (ptr == MAP_FAILED) {
        perror("DMA-BUF mmap");
        close(alloc.handle);
        close(dev);
        return 1;
    }

    printf("Mapped address: %p\n", ptr);

    memset(ptr, 0x5A, alloc.size);

    unsigned char *bytes = (unsigned char *)ptr;
    printf("Write/read test: %s\n",
           bytes[0] == 0x5A && bytes[4095] == 0x5A
               ? "PASSED" : "FAILED");

    munmap(ptr, alloc.size);
    close(alloc.handle);
    close(dev);

    puts("CMA probe finished.");
    return 0;
}
