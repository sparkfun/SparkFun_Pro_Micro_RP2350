

#ifndef _SFE_PICO_ALLOC_H_
#define _SFE_PICO_ALLOC_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif
    void *sfe_mem_malloc(size_t size);
    void sfe_mem_free(void *ptr);
    void *sfe_mem_realloc(void *ptr, size_t size);
    void *sfe_mem_calloc(size_t num, size_t size);
    size_t sfe_mem_max_free_size(void);
    // wrappers
    void *__wrap_malloc(size_t size);
    void __wrap_free(void *ptr);
    void *__wrap_realloc(void *ptr, size_t size);
    void *__wrap_calloc(size_t num, size_t size);
#ifdef __cplusplus
}
#endif

#endif