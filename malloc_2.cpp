#include <unistd.h>
#include <assert.h>
#include <string.h>


typedef struct MallocMetadata_t {
    size_t size;
    bool is_free;
    MallocMetadata_t* next;
    MallocMetadata_t* prev;
}*MallocMetadata;

static MallocMetadata head = NULL;
const size_t METADATA_SIZE=sizeof(MallocMetadata_t);

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
static void* allocate_new_block(size_t size, MallocMetadata prev);

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
        current->is_free = false;
        return (void *) (current + 1);
    }
    assert(current == NULL);
    return allocate_new_block(size, prev_meta_data);
}

void* scalloc(size_t num, size_t size){
    void* new_ptr = smalloc(num*size);
    if (NULL == new_ptr)
        return NULL;
    memset(new_ptr, 0, num*size);
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
    memmove(new_ptr, oldp, md->size);
    sfree(oldp);
    return new_ptr;
}

void sfree(void* p){
    MallocMetadata metadata=((MallocMetadata)p)-1;
    assert(!metadata->is_free);
    metadata->is_free=true;
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
    if(prev != NULL){
        prev->next = metadata;
    }
    return (void*)(metadata+1);
}

size_t _num_free_blocks(){
    MallocMetadata current = head;
    size_t count = 0;
    while (NULL != current){
        if (current->is_free)
            count++;
        current = current->next;
    }
    return count;
}

size_t _num_free_bytes(){
    MallocMetadata current = head;
    size_t count = 0;
    while (NULL != current){
        if (current->is_free)
            count += current->size;
        current = current->next;
    }
    return count;
}

size_t _num_allocated_blocks(){
    MallocMetadata current = head;
    size_t count = 0;
    while (NULL != current){
        count++;
        current = current->next;
    }
    return count;
}

size_t _num_allocated_bytes(){
    MallocMetadata current = head;
    size_t count = 0;
    while (NULL != current){
        count += current->size;
        current = current->next;
    }
    return count;
}

size_t _num_meta_data_bytes(){
    return _num_allocated_blocks()*METADATA_SIZE;
}


size_t _size_meta_data(){
    return METADATA_SIZE;
}
