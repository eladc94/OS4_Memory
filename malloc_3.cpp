#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>


typedef struct MallocMetadata_t {
    size_t size;
    bool is_free;
    MallocMetadata_t* next;
    MallocMetadata_t* prev;
}*MallocMetadata;



const size_t MIN_SPLIT=128;
const size_t MIN_MMAP_SIZE = 131072; //128KB
const size_t METADATA_SIZE=sizeof(MallocMetadata_t);
static MallocMetadata head = NULL;
static MallocMetadata mmap_head = NULL;

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
static void* new_mmap_node(size_t size);
static void* allocate_new_block(size_t size, MallocMetadata prev);
static void* split_block(MallocMetadata current,size_t size);
static void* change_wilderness_block(size_t size, MallocMetadata prev);
static void coalesce_blocks(MallocMetadata left,MallocMetadata right);


void* smalloc(size_t size) {
    if (size == 0 || size > MAX_SIZE)
        return NULL;
    if(size >= MIN_MMAP_SIZE){
        return new_mmap_node(size);
    }
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
        if(current->size >= (size+METADATA_SIZE+MIN_SPLIT))
            return split_block(current,size);
        current->is_free = false;
        return (void *) (current + 1);
    }
    assert(current == NULL);
    if (prev_meta_data!=NULL && prev_meta_data->is_free)
        return change_wilderness_block(size, prev_meta_data);
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
    if(size >= MIN_MMAP_SIZE){
        void* ptr = smalloc(size);
        MallocMetadata new_md = (MallocMetadata)ptr-1;
        MallocMetadata old_md = (MallocMetadata)oldp-1;
        if(old_md->size < new_md->size)
            memmove(ptr,oldp,old_md->size);
        else
            memmove(ptr,oldp,new_md->size);
        sfree(oldp);
        return ptr;
    }
    MallocMetadata md = (MallocMetadata)oldp-1;
    if(md->size >= size){
        md->is_free = false;
        if (md->size - size >= MIN_SPLIT+METADATA_SIZE)
            return split_block(md, size);
        else
            return oldp;
    }
    else if(md->next == NULL){
        return change_wilderness_block(size, md);
    }
    else if(md->prev != NULL && md->prev->is_free && (md->prev->size + md->size+METADATA_SIZE >= size)){
        MallocMetadata prev = md->prev;
        prev->next = md->next;
        prev->is_free = false;
        prev->size += md->size+METADATA_SIZE;
        memmove(prev+1, md+1, md->size);
        return split_block(prev, size);
    }
    else if(md->next != NULL && md->next->is_free && (md->next->size+md->size+METADATA_SIZE >= size)){
        MallocMetadata next = md->next;
        md->next = md->next->next;
        md->size += next->size+METADATA_SIZE;
        md->is_free = false;
        return split_block(md, size);
    }
    else if(md->prev != NULL && md->next != NULL && md->prev->is_free && md->next->is_free &&
            (md->prev->size+md->size+md->next->size+2*METADATA_SIZE >= size)){
        MallocMetadata next = md->next;
        MallocMetadata prev = md->prev;
        prev->next = next->next;
        prev->is_free = false;
        prev->size += md->size + next->size + 2*METADATA_SIZE;
        memmove(prev+1, md+1, md->size);
        return split_block(prev, size);
    }
    else {
        void *new_ptr = smalloc(size);
        if (NULL == new_ptr)
            return NULL;
        memmove(new_ptr, oldp, md->size);
        sfree(oldp);
        return new_ptr;
    }
}

void sfree(void* p){
    MallocMetadata metadata=((MallocMetadata)p)-1;
    assert(!metadata->is_free);
    if(metadata->size>=MIN_MMAP_SIZE){
        if(metadata->prev == NULL) {
            if (metadata->next != NULL)
                metadata->next->prev = NULL;
            mmap_head = metadata->next;
        }
        else{
            metadata->prev->next=metadata->next;
        }
        if(metadata->next != NULL)
            metadata->next->prev = metadata->prev;
        int error_check = munmap((void*)(metadata),metadata->size+METADATA_SIZE);
        assert(error_check == 0);
        return;
    }
    metadata->is_free=true;
    if(metadata->next!=NULL && metadata->next->is_free)
        coalesce_blocks(metadata,metadata->next);
    if(metadata->prev!=NULL && metadata->prev->is_free)
        coalesce_blocks(metadata->prev,metadata);
}

void* new_mmap_node(size_t size){
    MallocMetadata ptr =(MallocMetadata)mmap(NULL,size+METADATA_SIZE,PROT_READ|PROT_WRITE,
            MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    ptr->size=size;
    ptr->is_free=false;
    ptr->next=NULL;
    if(mmap_head == NULL){
        ptr->prev = NULL;
        mmap_head=ptr;
        return (void*)(ptr+1);
    }
    MallocMetadata current = mmap_head;
    MallocMetadata prev = NULL;
    while(current != NULL){
        prev = current;
        current = current->next;
    }
    prev->next=ptr;
    ptr->prev=prev;
    return (void*)(ptr+1);
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

void* change_wilderness_block(size_t size, MallocMetadata prev){
    MallocMetadata metadata;
    size_t toAdd = size - prev->size;
    void* ptr = sbrk(toAdd);
    if(ptr == (void*)-1){
        return NULL;
    }
    metadata = prev;
    metadata->size=size;
    metadata->is_free=false;
    return (void*)(metadata+1);
}

void* split_block(MallocMetadata current, size_t size){
    if(current->size < size + METADATA_SIZE + MIN_SPLIT)
        return (void*)(current+1);
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
    return (void*)(current+1);
}

void coalesce_blocks(MallocMetadata left, MallocMetadata right){
    if(left == NULL || right == NULL)
        return;
    assert(left->is_free && right->is_free);
    size_t new_size = left->size+right->size+METADATA_SIZE;
    left->next=right->next;
    left->size=new_size;
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
    current = mmap_head;
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
    current = mmap_head;
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
