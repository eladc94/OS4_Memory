#include <unistd.h>
#include <assert.h>


typedef struct MallocMetadata_t {
    size_t size;
    bool is_free;
    MallocMetadata_t* next;
    MallocMetadata_t* prev;
}*MallocMetadata;


const size_t MAX_SIZE=100000000;
static MallocMetadata head = NULL;
static size_t num_free_blocks = 0;
static size_t num_free_bytes = 0;
static size_t num_allocated_blocks = 0;
static size_t num_allocates_bytes = 0;
static size_t num_meta_data_bytes = 0;


size_t _size_meta_data();
void* __allocate_new_block(size_t size,MallocMetadata prev);

void* smalloc(size_t size){
    if(size == 0 || size > MAX_SIZE)
        return NULL;
    MallocMetadata metadata;
    if (head == NULL){
        __allocate_new_block(size,NULL);
    }
    MallocMetadata current = head;
    MallocMetadata prev_meta_data = NULL;
    while(current != NULL){
        if((!current->is_free) || (current->size<size)){
            prev_meta_data=current;
            current=current->next;
            continue;
        }
        assert((current->is_free)&&(current->size>=size));
        current->is_free=false;
        num_free_blocks--;
        num_free_bytes-=size;
        assert(num_free_blocks>=0 && num_free_bytes >=0);
        return (void*)(current+1);
    }
    assert(current == NULL);
    return __allocate_new_block(size,prev_meta_data);
}

void* scalloc(size_t num, size_t size){}
void sfree(void* p){
    MallocMetadata metadata=((MallocMetadata)p)-1;
    assert(!metadata->is_free);
    metadata->is_free=true;
    num_free_blocks++;
    num_free_bytes+=metadata->size;
}
void* srealloc(void* oldp, size_t size){}
size_t _num_free_blocks(){}
size_t _num_free_bytes(){}
size_t _num_allocated_blocks(){}
size_t _num_allocated_bytes(){}
size_t _num_meta_data_bytes(){}
size_t _size_meta_data(){
    return sizeof(MallocMetadata_t);
}

void* __allocate_new_block(size_t size, MallocMetadata prev_meta_data){
    MallocMetadata metadata;
    void* ptr=sbrk(size+_size_meta_data());
    if(ptr == (void*)-1){
        return NULL;
    }
    metadata=(MallocMetadata)ptr;
    metadata->size=size;
    metadata->is_free=false;
    metadata->next=NULL;
    metadata->prev=prev_meta_data;
    head=metadata;
    num_allocated_blocks++;
    num_allocates_bytes+=size;
    //elad veiza
    num_meta_data_bytes+=_size_meta_data();
    if(prev_meta_data != NULL){
        prev_meta_data->next = metadata;
    }
    return (void*)(metadata+1);
}
