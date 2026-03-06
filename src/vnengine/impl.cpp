// Implement header-only libraries

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

#define VMA_LOG(preface, format, ...)                                          \
  do {                                                                         \
    printf(preface format "\n", __VA_ARGS__);                                  \
  } while (false)

// Always log leaks
#define VMA_LEAK_LOG_FORMAT(format, ...)                                       \
  VMA_LOG("[vma: leak] ", format, __VA_ARGS__)

// Don't log allocations unless requested, as these are very verbose
#ifdef VN_LOGALLOCATIONS
#define VMA_DEBUG_LOG_FORMAT(format, ...)                                      \
  VMA_LOG("[vma: info] ", format, __VA_ARGS__)
#endif

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
