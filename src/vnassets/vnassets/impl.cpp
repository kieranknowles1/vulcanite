// Implement header-only libraries

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#pragma GCC diagnostic push
// We normally compile with this as an error. STB image has fallthroughs that
// we need to ignore
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#include <stb_image.h>
#pragma GCC diagnostic pop
