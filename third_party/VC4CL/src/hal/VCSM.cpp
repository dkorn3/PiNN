/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "VCSM.h"

#include "userland.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

using namespace vc4cl;

// Current Raspberry Pi vcsm-cma userspace ABI.
#define VC_SM_CMA_MAGIC_TYPE 'J'
#define VC_SM_CMA_CMD_ALLOC 0x5A
#define VC_SM_CMA_RESOURCE_NAME 32

struct vc_sm_cma_ioctl_alloc
{
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
    _IOR(VC_SM_CMA_MAGIC_TYPE, VC_SM_CMA_CMD_ALLOC, struct vc_sm_cma_ioctl_alloc)

VCSM::VCSM(bool useCMA)
{
    if(useCMA)
    {
        int fd = open("/dev/vcsm-cma", O_RDWR | O_CLOEXEC);
        if(fd < 0)
            throw std::runtime_error("Failed to open /dev/vcsm-cma!");

        close(fd);
        usesCMA = true;
    }
    else
    {
        if(vcsm_init_ex(0, -1) != 0)
            throw std::runtime_error("Failed to initialize legacy VCSM!");

        usesCMA = true;
        int fd = open("/dev/vcsm", O_RDWR | O_CLOEXEC);
        if(fd >= 0)
        {
            usesCMA = false;
            close(fd);
        }
    }

    DEBUG_LOG(
        DebugLevel::SYSCALL,
        std::cout << "[VC4CL] VCSM initialized: "
                  << (usesCMA ? "vcsm-cma" : "vcsm") << std::endl)
}

VCSM::~VCSM()
{
    if(!usesCMA)
        vcsm_exit();

    DEBUG_LOG(DebugLevel::SYSCALL, std::cout << "[VC4CL] VCSM shut down" << std::endl)
}

static VCSM_CACHE_TYPE_T toCacheType(CacheType type)
{
    switch(type)
    {
    case CacheType::UNCACHED:
        return static_cast<VCSM_CACHE_TYPE_T>(VCSM_CACHE_TYPE_PINNED | VCSM_CACHE_TYPE_NONE);
    case CacheType::HOST_CACHED:
        return static_cast<VCSM_CACHE_TYPE_T>(VCSM_CACHE_TYPE_PINNED | VCSM_CACHE_TYPE_HOST);
    case CacheType::GPU_CACHED:
        return static_cast<VCSM_CACHE_TYPE_T>(VCSM_CACHE_TYPE_PINNED | VCSM_CACHE_TYPE_VC);
    case CacheType::BOTH_CACHED:
    default:
        return static_cast<VCSM_CACHE_TYPE_T>(VCSM_CACHE_TYPE_PINNED | VCSM_CACHE_TYPE_HOST_AND_VC);
    }
}

std::unique_ptr<DeviceBuffer> VCSM::allocateBuffer(
    const std::shared_ptr<SystemAccess>& system, unsigned sizeInBytes,
    const std::string& name, CacheType cacheType)
{
    if(usesCMA)
    {
        if(sizeInBytes == 0 ||
            sizeInBytes > std::numeric_limits<uint32_t>::max() - (PAGE_ALIGNMENT - 1))
            return nullptr;

        const uint32_t alignedSize =
            (sizeInBytes + PAGE_ALIGNMENT - 1) & ~(PAGE_ALIGNMENT - 1);

        int deviceFd = open("/dev/vcsm-cma", O_RDWR | O_CLOEXEC);
        if(deviceFd < 0)
        {
            perror("[VC4CL] open /dev/vcsm-cma");
            return nullptr;
        }

        vc_sm_cma_ioctl_alloc alloc{};
        alloc.size = alignedSize;
        alloc.num = 1;
        alloc.cached = 0;
        std::strncpy(
            reinterpret_cast<char*>(alloc.name),
            name.c_str(),
            sizeof(alloc.name) - 1);

        if(ioctl(deviceFd, VC_SM_CMA_IOCTL_MEM_ALLOC, &alloc) < 0)
        {
            perror("[VC4CL] CMA allocation ioctl");
            close(deviceFd);
            return nullptr;
        }

        close(deviceFd);

        if(alloc.handle < 0 || alloc.dma_addr == 0 ||
            alloc.dma_addr > std::numeric_limits<uint32_t>::max())
        {
            std::fprintf(stderr, "[VC4CL] Invalid CMA allocation result\n");
            if(alloc.handle >= 0)
                close(alloc.handle);
            return nullptr;
        }

        void* hostPointer = mmap(
            nullptr, alignedSize,
            PROT_READ | PROT_WRITE,
            MAP_SHARED, alloc.handle, 0);

        if(hostPointer == MAP_FAILED)
        {
            perror("[VC4CL] CMA DMA-BUF mmap");
            close(alloc.handle);
            return nullptr;
        }

        DevicePointer qpuPointer{static_cast<uint32_t>(alloc.dma_addr)};

        DEBUG_LOG(
            DebugLevel::MEMORY,
            std::cout << "Allocated " << alignedSize
                      << " bytes using vcsm-cma: fd " << alloc.handle
                      << ", device address " << std::hex << "0x" << qpuPointer
                      << ", host address " << hostPointer << std::dec << std::endl)

        return std::unique_ptr<DeviceBuffer>{
            new DeviceBuffer(system, static_cast<uint32_t>(alloc.handle),
                qpuPointer, hostPointer, alignedSize)};
    }

    // Original legacy VCSM allocation path.
    auto handle = vcsm_malloc_cache(sizeInBytes, toCacheType(cacheType), name.data());
    if(handle == 0)
    {
        DEBUG_LOG(DebugLevel::SYSCALL,
            std::cout << "[VC4CL] Failed to allocate VCSM handle for "
                      << sizeInBytes << " of data!" << std::endl)
        return nullptr;
    }

    auto hostPointer = vcsm_lock(handle);
    if(hostPointer == nullptr)
    {
        DEBUG_LOG(DebugLevel::SYSCALL,
            std::cout << "[VC4CL] Failed to lock VCSM handle: " << handle << std::endl)
        vcsm_free(handle);
        return nullptr;
    }

    DevicePointer qpuPointer{vcsm_vc_addr_from_hdl(handle)};
    if(static_cast<uint32_t>(qpuPointer) == 0)
    {
        DEBUG_LOG(DebugLevel::SYSCALL,
            std::cout << "[VC4CL] Failed to get bus address for VCSM handle: "
                      << handle << std::endl)
        vcsm_unlock_ptr(hostPointer);
        vcsm_free(handle);
        return nullptr;
    }

    DEBUG_LOG(DebugLevel::MEMORY,
        std::cout << "Allocated " << sizeInBytes << " bytes of buffer: handle "
                  << handle << ", device address " << std::hex << "0x" << qpuPointer
                  << ", host address " << hostPointer << std::dec << std::endl)

    return std::unique_ptr<DeviceBuffer>{
        new DeviceBuffer(system, handle, qpuPointer, hostPointer, sizeInBytes)};
}

bool VCSM::deallocateBuffer(const DeviceBuffer* buffer)
{
    if(usesCMA)
    {
        bool success = true;

        if(buffer->hostPointer &&
            munmap(buffer->hostPointer, buffer->size) != 0)
        {
            perror("[VC4CL] CMA munmap");
            success = false;
        }

        if(close(static_cast<int>(buffer->memHandle)) != 0)
        {
            perror("[VC4CL] CMA DMA-BUF close");
            success = false;
        }

        DEBUG_LOG(DebugLevel::MEMORY,
            std::cout << "Deallocated CMA buffer: fd " << buffer->memHandle
                      << ", device address " << std::hex << "0x" << buffer->qpuPointer
                      << std::dec << std::endl)

        return success;
    }

    if(buffer->hostPointer)
    {
        if(int status = vcsm_unlock_ptr(buffer->hostPointer))
        {
            errno = -status;
            perror("[VC4CL] Error in vcsm_unlock_ptr");
            return false;
        }
    }

    vcsm_free(buffer->memHandle);

    DEBUG_LOG(DebugLevel::MEMORY,
        std::cout << "Deallocated " << buffer->size << " bytes of buffer: handle "
                  << buffer->memHandle << ", device address " << std::hex
                  << "0x" << buffer->qpuPointer << ", host address "
                  << buffer->hostPointer << std::dec << std::endl)

    return true;
}

bool VCSM::flushCPUCache(const std::vector<const DeviceBuffer*>& buffers)
{
    // vcsm-cma allocates coherent DMA memory and imports it into VideoCore
    // as non-cached memory. No legacy VCSM cache operation is needed.
    if(usesCMA)
        return true;

    auto data = [&]() {
        auto bufferSize = sizeof(vcsm_user_clean_invalid2_s) +
            buffers.size() * sizeof(vcsm_user_clean_invalid2_s::vcsm_user_clean_invalid2_block_s);

        auto* raw = std::malloc(bufferSize);
        if(!raw)
            return std::unique_ptr<vcsm_user_clean_invalid2_s, decltype(&free)>{
                nullptr, free};

        std::memset(raw, 0, bufferSize);
        auto* ptr = reinterpret_cast<vcsm_user_clean_invalid2_s*>(raw);
        ptr->op_count = static_cast<uint8_t>(buffers.size());

        return std::unique_ptr<vcsm_user_clean_invalid2_s, decltype(&free)>{
            ptr, free};
    }();

    if(!data)
        return false;

    for(uint8_t i = 0; i < data->op_count; ++i)
    {
        data->s[i].invalidate_mode = VC_SM_CACHE_OP_FLUSH;
        data->s[i].block_count = 1;
        data->s[i].start_address = buffers[i]->hostPointer;
        data->s[i].block_size = buffers[i]->size;
        data->s[i].inter_block_stride = 0;
    }

    return vcsm_clean_invalid2(data.get()) == 0;
}

uint32_t VCSM::getTotalCMAMemory()
{
    static const uint32_t cmaMemory = []() {
        std::ifstream fis("/proc/meminfo");
        std::string line;
        while(std::getline(fis, line))
        {
            if(line.find("CmaTotal") != std::string::npos)
            {
                auto start = line.find_first_of("0123456789");
                if(start != std::string::npos)
                    return static_cast<uint32_t>(
                        strtoul(line.data() + start, nullptr, 0)) * 1024;
                break;
            }
        }
        return uint32_t{0};
    }();

    return cmaMemory;
}

bool VCSM::readValue(SystemQuery query, uint32_t& output) noexcept
{
    switch(query)
    {
    case SystemQuery::TOTAL_GPU_MEMORY_IN_BYTES:
        if(isUsingCMA())
        {
            output = getTotalCMAMemory();
            return true;
        }
        return false;
    default:
        return false;
    }
}
