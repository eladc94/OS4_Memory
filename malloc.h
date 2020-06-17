
#ifndef OS4_MEMORY_MALLOC_H

#define OS4_MEMORY_MALLOC_H
const size_t MAX_SIZE=100000000;
typedef struct MallocMetadata_t *MallocMetadata;
void* smalloc(size_t size);
void* scalloc(size_t num,size_t size);
void* srealloc(void* old,size_t size);
void sfree(void* p);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_metadata_bytes();
size_t _size_meta_data();
#endif //OS4_MEMORY_MALLOC_H
