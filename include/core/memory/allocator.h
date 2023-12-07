#ifndef __JPOD_SERVICE_CORE_MEMORY_ALLOCATOR__
#define __JPOD_SERVICE_CORE_MEMORY_ALLOCATOR__

namespace core::memory
{
    template <typename T>
    class ArenaAllocator
    {
    public:
        T *allocate(std::size_t size)
        {
        }
        void deallocate(T *ptr, std::size_t size)
        {
        }
    };
}
#endif // __JPOD_SERVICE_CORE_MEMORY_ALLOCATOR__