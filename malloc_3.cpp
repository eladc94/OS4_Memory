#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "malloc.h"

static MallocMetadata head = NULL;
static size_t num_free_blocks = 0;
static size_t num_free_bytes = 0;
static size_t num_allocated_blocks = 0;
static size_t num_allocated_bytes = 0;
static size_t num_meta_data_bytes = 0;

struct MallocMetadata_t {
    size_t size;
    bool is_free;
    MallocMetadata_t* next;
    MallocMetadata_t* prev;
};

void* smalloc(size_t size) {
    if (size == 0 || size > MAX_SIZE)
        return NULL;
    MallocMetadata metadata;
    if (head == NULL) {
        return allocate_new_block(size, NULL);
    }
    MallocMetadata current = head;
    MallocMetadata prev_meta_data = NULL;
    while (current != NULL) {
        if ((!current->is_free) || (current->size < size)) {
            prev_meta_data = current;
            current = current->next;
            continue;
        }
        assert((current->is_free) && (current->size >= size));
        current->is_free = false;
        num_free_blocks--;
        num_free_bytes -= current->size;
        assert(num_free_blocks >= 0 && num_free_bytes >= 0);
        return (void *) (current + 1);
    }
    assert(current == NULL);
    return allocate_new_block(size, prev_meta_data);
}

void* scalloc(size_t num, size_t size){
    void* new_ptr = smalloc(num*size);
    if (NULL == new_ptr)
        return NULL;
    memset(new_ptr, 0, size);
    return new_ptr;
}

void* srealloc(void* oldp, size_t size){
    if (NULL == oldp)
        return smalloc(size);
    MallocMetadata md = (MallocMetadata)oldp-1;
    if(md->size >= size){
        md->is_free = false;
        return oldp;
    }
    void* new_ptr = smalloc(size);
    if (NULL == new_ptr)
        return NULL;
    memcpy(new_ptr, oldp, md->size);
    sfree(oldp);
    return new_ptr;
}

void sfree(void* p){
    MallocMetadata metadata=((MallocMetadata)p)-1;
    assert(!metadata->is_free);
    metadata->is_free=true;
    num_free_blocks++;
    num_free_bytes+=metadata->size;
}

void* allocate_new_block(size_t size, MallocMetadata prev){
    MallocMetadata metadata;
    void* ptr=sbrk(size+_size_meta_data());
    if(ptr == (void*)-1){
        return NULL;
    }
    metadata=(MallocMetadata)ptr;
    metadata->size=size;
    metadata->is_free=false;
    metadata->next=NULL;
    metadata->prev=prev;
    if(NULL == prev)
        head = metadata;
    num_allocated_blocks++;
    num_allocated_bytes+=size;
    num_meta_data_bytes+=_size_meta_data();
    if(prev != NULL){
        prev->next = metadata;
    }
    return (void*)(metadata+1);
}

void split_and_allocate_block(size_t size,MallocMetadata prev,MallocMetadata next){
//elad beitza
}

size_t _num_free_blocks(){
    return num_free_blocks;
}

size_t _num_free_bytes(){
    return num_free_bytes;
}

size_t _num_allocated_blocks(){
    return num_allocated_blocks;
}

size_t _num_allocated_bytes(){
    return num_allocated_bytes;
}

size_t _num_metadata_bytes(){
    return num_meta_data_bytes;
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata_t);
}
