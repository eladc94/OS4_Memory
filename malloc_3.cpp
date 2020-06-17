#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "malloc.h"

const int MIN_SPLIT=128;
const int METADATA_SIZE=_size_meta_data();
static MallocMetadata head = NULL;
static size_t num_free_blocks = 0;
static size_t num_free_bytes = 0;
static size_t num_allocated_blocks = 0;
static size_t num_allocated_bytes = 0;
static size_t num_meta_data_bytes = 0;

static void* allocate_new_block(size_t size, MallocMetadata prev);
static void* split_block(MallocMetadata current,size_t size);
static void coalesce_blocks(MallocMetadata left,MallocMetadata right);

struct MallocMetadata_t {
    size_t size;
    bool is_free;
    MallocMetadata_t* next;
    MallocMetadata_t* prev;
};

void* smalloc(size_t size) {
    if (size == 0 || size > MAX_SIZE)
        return NULL;
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
        if(current->size > (size+METADATA_SIZE+MIN_SPLIT))
            return split_block(current,size);
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
    if(metadata->next!=NULL && metadata->next->is_free)
        coalesce_blocks(metadata,metadata->next);
    if(metadata->next!=NULL && metadata->prev)
        coalesce_blocks(metadata->prev,metadata);
}

void* allocate_new_block(size_t size, MallocMetadata prev){
    MallocMetadata metadata;
    void* ptr=sbrk(size+METADATA_SIZE);
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
    num_meta_data_bytes+=METADATA_SIZE;
    if(prev != NULL){
        prev->next = metadata;
    }
    return (void*)(metadata+1);
}

void* split_block(MallocMetadata current, size_t size){
    size_t start_size = current->size;
    MallocMetadata temp_next = current->next;
    current->size = size;
    current->is_free = false;
    char* ptr = (char*) (current+1);
    MallocMetadata new_metadata = (MallocMetadata) (ptr+size);
    new_metadata->size= (start_size-size) - METADATA_SIZE;
    new_metadata->is_free = true;
    current->next = new_metadata;
    new_metadata->prev = current;
    new_metadata->next = temp_next;
    num_allocated_blocks++;
    num_free_blocks-=(size+METADATA_SIZE);
    num_meta_data_bytes+=METADATA_SIZE;
    return (void*)(current+1);
}

void coalesce_blocks(MallocMetadata left, MallocMetadata right){
    if(left == NULL || right == NULL)
        return;
    assert(left->is_free && right->is_free);
    size_t new_size = left->size+right->size+METADATA_SIZE;
    left->next=right->next;
    left->size=new_size;
    num_free_blocks--;
    num_meta_data_bytes-=METADATA_SIZE;
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
