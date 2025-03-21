/*
 * Copyright (c) 2024 EdgeImpulse Inc.
 *
 * Generated by Edge Impulse and licensed under the applicable Edge Impulse
 * Terms of Service. Community and Professional Terms of Service
 * (https://edgeimpulse.com/legal/terms-of-service) or Enterprise Terms of
 * Service (https://edgeimpulse.com/legal/enterprise-terms-of-service),
 * according to your product plan subscription (the “License”).
 *
 * This software, documentation and other associated files (collectively referred
 * to as the “Software”) is a single SDK variation generated by the Edge Impulse
 * platform and requires an active paid Edge Impulse subscription to use this
 * Software for any purpose.
 *
 * You may NOT use this Software unless you have an active Edge Impulse subscription
 * that meets the eligibility requirements for the applicable License, subject to
 * your full and continued compliance with the terms and conditions of the License,
 * including without limitation any usage restrictions under the applicable License.
 *
 * If you do not have an active Edge Impulse product plan subscription, or if use
 * of this Software exceeds the usage limitations of your Edge Impulse product plan
 * subscription, you are not permitted to use this Software and must immediately
 * delete and erase all copies of this Software within your control or possession.
 * Edge Impulse reserves all rights and remedies available to enforce its rights.
 *
 * Unless required by applicable law or agreed to in writing, the Software is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing
 * permissions, disclaimers and limitations under the License.
 */
#ifndef _EIDSP_MEMORY_H_
#define _EIDSP_MEMORY_H_

// clang-format off
#include <functional>
#include <stdio.h>
#include <memory>
#include "../porting/ei_classifier_porting.h"
#include "edge-impulse-sdk/classifier/ei_aligned_malloc.h"
#include "config.hpp"

extern size_t ei_memory_in_use;
extern size_t ei_memory_peak_use;

#if EIDSP_PRINT_ALLOCATIONS == 1
#define ei_dsp_printf           printf
#else
#define ei_dsp_printf           (void)
#endif

typedef std::unique_ptr<void, std::function<void(void*)>> ei_unique_ptr_t;

// deprecated, use the class ei_tracked_unique_ptr below instead
// this version will NOT track memory usage
#define EI_ALLOCATE_AUTO_POINTER(ptr, size) \
    ptr = static_cast<decltype(ptr)>(ei_calloc(size,sizeof(*ptr))); \
    ei_unique_ptr_t __ptr__(ptr,ei_free);

#define EI_ERR_AND_RETURN_ON_NULL(ptr,code) \
    if( ! (ptr) ) { \
        ei_printf("Null check failed\n"); \
        return code; \
    }

namespace ei {

/**
 * These are macros used to track allocations when running DSP processes.
 * Enable memory tracking through the EIDSP_TRACK_ALLOCATIONS macro.
 */

#if EIDSP_TRACK_ALLOCATIONS
    /**
     * Register a manual allocation (malloc or calloc).
     * Typically you want to use ei::matrix_t types, as they keep track automatically.
     * @param bytes Number of bytes allocated
     */
    #define ei_dsp_register_alloc_internal(fn, file, line, bytes, ptr) \
        ei_memory_in_use += bytes; \
        if (ei_memory_in_use > ei_memory_peak_use) { \
            ei_memory_peak_use = ei_memory_in_use; \
        } \
        ei_dsp_printf("alloc %lu bytes (in_use=%lu, peak=%lu) (%s@ %s:%d) %p\n", \
            (unsigned long)bytes, (unsigned long)ei_memory_in_use, (unsigned long)ei_memory_peak_use, fn, file, line, ptr);

    /**
     * Register a matrix allocation. Don't call this function yourself,
     * matrices already track this automatically.
     * @param rows Number of rows
     * @param cols Number of columns
     * @param type_size Size of the data type
     */
    #define ei_dsp_register_matrix_alloc_internal(fn, file, line, rows, cols, type_size, ptr) \
        ei_memory_in_use += (rows * cols * type_size); \
        if (ei_memory_in_use > ei_memory_peak_use) { \
            ei_memory_peak_use = ei_memory_in_use; \
        } \
        ei_dsp_printf("alloc matrix %lu x %lu = %lu bytes (in_use=%lu, peak=%lu) (%s@ %s:%d) %p\n", \
            (unsigned long)rows, (unsigned long)cols, (unsigned long)(rows * cols * type_size), (unsigned long)ei_memory_in_use, \
                (unsigned long)ei_memory_peak_use, fn, file, line, ptr);

    /**
     * Register free'ing manually allocated memory (allocated through malloc/calloc)
     * @param bytes Number of bytes free'd
     */
    #define ei_dsp_register_free_internal(fn, file, line, bytes, ptr) \
        ei_memory_in_use -= bytes; \
        ei_dsp_printf("free %lu bytes (in_use=%lu, peak=%lu) (%s@ %s:%d) %p\n", \
            (unsigned long)bytes, (unsigned long)ei_memory_in_use, (unsigned long)ei_memory_peak_use, fn, file, line, ptr);

    /**
     * Register a matrix free. Don't call this function yourself,
     * matrices already track this automatically.
     * @param rows Number of rows
     * @param cols Number of columns
     * @param type_size Size of the data type
     */
    #define ei_dsp_register_matrix_free_internal(fn, file, line, rows, cols, type_size, ptr) \
        ei_memory_in_use -= (rows * cols * type_size); \
        ei_dsp_printf("free matrix %lu x %lu = %lu bytes (in_use=%lu, peak=%lu) (%s@ %s:%d) %p\n", \
            (unsigned long)rows, (unsigned long)cols, (unsigned long)(rows * cols * type_size), \
                (unsigned long)ei_memory_in_use, (unsigned long)ei_memory_peak_use, fn, file, line, ptr);

    #define ei_dsp_register_alloc(...) ei_dsp_register_alloc_internal(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_register_matrix_alloc(...) ei_dsp_register_matrix_alloc_internal(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_register_free(...) ei_dsp_register_free_internal(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_register_matrix_free(...) ei_dsp_register_matrix_free_internal(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_malloc(...) memory::ei_wrapped_malloc(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_calloc(...) memory::ei_wrapped_calloc(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_free(...) memory::ei_wrapped_free(__func__, __FILE__, __LINE__, __VA_ARGS__)

    #define EI_DSP_MATRIX(name, ...) matrix_t name(__VA_ARGS__, NULL, __func__, __FILE__, __LINE__); if (!name.buffer) { EIDSP_ERR(EIDSP_OUT_OF_MEM); }
    #define EI_DSP_MATRIX_B(name, ...) matrix_t name(__VA_ARGS__, __func__, __FILE__, __LINE__); if (!name.buffer) { EIDSP_ERR(EIDSP_OUT_OF_MEM); }
    #define EI_DSP_QUANTIZED_MATRIX(name, ...) quantized_matrix_t name(__VA_ARGS__, NULL, __func__, __FILE__, __LINE__); if (!name.buffer) { EIDSP_ERR(EIDSP_OUT_OF_MEM); }
    #define EI_DSP_QUANTIZED_MATRIX_B(name, ...) quantized_matrix_t name(__VA_ARGS__, __func__, __FILE__, __LINE__); if (!name.buffer) { EIDSP_ERR(EIDSP_OUT_OF_MEM); }
#else
    #define ei_dsp_register_alloc(...) (void)0
    #define ei_dsp_register_matrix_alloc(...) (void)0
    #define ei_dsp_register_free(...) (void)0
    #define ei_dsp_register_matrix_free(...) (void)0
    #define ei_dsp_malloc ei_malloc
    #define ei_dsp_calloc ei_calloc
    #define ei_dsp_free(ptr, size) ei_free(ptr)
    #define EI_DSP_MATRIX(name, ...) matrix_t name(__VA_ARGS__); if (!name.buffer) { EIDSP_ERR(EIDSP_OUT_OF_MEM); }
    #define EI_DSP_MATRIX_B(name, ...) matrix_t name(__VA_ARGS__); if (!name.buffer) { EIDSP_ERR(EIDSP_OUT_OF_MEM); }
    #define EI_DSP_QUANTIZED_MATRIX(name, ...) quantized_matrix_t name(__VA_ARGS__); if (!name.buffer) { EIDSP_ERR(EIDSP_OUT_OF_MEM); }
    #define EI_DSP_QUANTIZED_MATRIX_B(name, ...) quantized_matrix_t name(__VA_ARGS__); if (!name.buffer) { EIDSP_ERR(EIDSP_OUT_OF_MEM); }
#endif

#if EIDSP_TRACK_ALLOCATIONS
class memory {


public:
    /**
     * Allocate a new block of memory
     * @param size The size of the memory block, in bytes.
     */
    static void *ei_wrapped_malloc(const char *fn, const char *file, int line, size_t size) {
        void *ptr = ei_malloc(size);
        if (ptr) {
            ei_dsp_register_alloc_internal(fn, file, line, size, ptr);
        }
        return ptr;
    }

    /**
     * Allocates a block of memory for an array of num elements, each of them size bytes long,
     * and initializes all its bits to zero.
     * @param num Number of elements to allocate
     * @param size Size of each element
     */
    static void *ei_wrapped_calloc(const char *fn, const char *file, int line, size_t num, size_t size) {
        void *ptr = ei_calloc(num, size);
        if (ptr) {
            ei_dsp_register_alloc_internal(fn, file, line, num * size, ptr);
        }
        return ptr;
    }

    /**
     * Deallocate memory previously allocated by a call to calloc, malloc, or realloc.
     * @param ptr Pointer to a memory block previously allocated with malloc, calloc or realloc.
     * @param size Size of the block of memory previously allocated.
     */
    static void ei_wrapped_free(const char *fn, const char *file, int line, void *ptr, size_t size) {
        ei_free(ptr);
        ei_dsp_register_free_internal(fn, file, line, size, ptr);
    }
};
#endif // #if EIDSP_TRACK_ALLOCATIONS

// This needs to be a real function so I can bind with a lambda
__attribute__((unused)) static void ei_dsp_free_func(void *ptr, size_t size) {
    ei_free(ptr);
#if EIDSP_TRACK_ALLOCATIONS
    ei_dsp_register_free_internal("unique_ptr free", "", 0, size, ptr);
#endif
}

#if EIDSP_TRACK_ALLOCATIONS
/**
* @brief Get the tracked unique ptr object NOTE EI_MAKE_TRACKED_POINTER is easier to use (it wraps this func)
*
* @param ptr_in The type is a hack.  It should be void**,
                but then you'd have to typecast something like float** explicitly.
                This is a pointer to a pointer where we should right the malloc'd addr
* @param size Desired size of the memory block, in BYTES.
* @return ei_tracked_unique_ptr
*/
static ei_unique_ptr_t make_tracked_unique_ptr(void* ptr_in, size_t size) {
    auto ptr = reinterpret_cast<void**>(ptr_in);
    *ptr = ei_dsp_malloc(size);
    return ei_unique_ptr_t(*ptr, [size](void *ptr) {
        ei_free(ptr);
        ei_dsp_register_free_internal("unique_ptr", "", 0, size, ptr);
    });
}
#else

/**
* @brief Get the tracked unique ptr object. NOTE EI_MAKE_TRACKED_POINTER is easier to use (it wraps this func)
*
* @param ptr_in The type is a hack.  It should be void**,
                but then you'd have to typecast something like float** explicitly.
                This is a pointer to a pointer where we should right the malloc'd addr
* @param size Desired size of the memory block, in BYTES.
* @return ei_tracked_unique_ptr
*/
__attribute__((warn_unused_result)) __attribute__((unused))
static ei_unique_ptr_t make_tracked_unique_ptr(void* ptr_in, size_t size)
{
    auto ptr = reinterpret_cast<void**>(ptr_in);
    *ptr = ei_malloc(size);
    return ei_unique_ptr_t(*ptr, ei_free);
}
#endif

/*
 * @brief Make a unique ptr that supports memory tracking
 * @param ptr A pointer that will be written with the malloc'd address
 * @param size Desired size of the memory block, in ITEMS.
 */
#define EI_MAKE_TRACKED_POINTER(ptr, size) ei::make_tracked_unique_ptr(&ptr, sizeof(*ptr)*size);

} // namespace ei

// clang-format on
#endif // _EIDSP_MEMORY_H_
