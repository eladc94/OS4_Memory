#include <unistd.h>



typedef struct MallocMetadata_t {
    size_t size;
    bool is_free;
    MallocMetadata_t* next;
    MallocMetadata_t* prev;
}*MallocMetadata;


static MallocMetadata head = NULL;
static size_t num_free_blocks = 0;
static size_t num_free_bytes = 0;
static size_t num_allocated_blocks = 0;
static size_t num_allocates_bytes = 0;
static size_t num_meta_data_bytes = 0;
static size_t size_meta_data = 0;


void* smalloc(size_t size){}
void* scalloc(size_t num, size_t size){}
void sfree(void* p){}
void* srealloc(void* oldp, size_t size){}
size_t _num_free_blocks(){}
size_t _num_free_bytes(){}
size_t _num_allocated_blocks(){}
size_t _num_allocated_bytes(){}
size_t _num_meta_data_bytes(){}
size_t _size_meta_data(){}