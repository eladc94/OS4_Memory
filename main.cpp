#include <iostream>
#include "assert.h"
#include "malloc.h"
const size_t SIZE=_size_meta_data();
struct MallocMetadata_t {
    size_t size;
    bool is_free;
    MallocMetadata_t* next;
    MallocMetadata_t* prev;
};

int main() {
    MallocMetadata elad;
    std::cout<<sizeof(*elad)<<std::endl;
    int* test=(int*)smalloc(sizeof(int)*100);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 400);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_metadata_bytes() == SIZE);
    sfree(test);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 400);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 400);
    assert(_num_metadata_bytes() == SIZE);
    test = (int*)scalloc(100,4);
    for(int i=0;i<100;i++)
        assert(test[i] == 0);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 400);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_metadata_bytes() == SIZE);
    sfree(test);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 400);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 400);
    assert(_num_metadata_bytes() == SIZE);
    test = (int*)scalloc(10,4);
    for(int i=0;i<10;i++)
        assert(test[i] == 0);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 368);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 328);
    assert(_num_metadata_bytes() == 2*SIZE);
    sfree(test);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 400);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 400);
    assert(_num_metadata_bytes() == SIZE);
    test = (int*)scalloc(200,4);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 800);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_metadata_bytes() == SIZE);
    for(int i=0;i<200;i++)
        assert(test[i] == 0);
    sfree(test);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 800);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 800);
    assert(_num_metadata_bytes() == SIZE);
    test = (int*)smalloc(800);
    int* temp = test;
    test = (int*)srealloc(test,100);
    assert(temp == test);
    for(int i =0;i< 25;i++)
        test[i]=i;
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 768);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 668);
    assert(_num_metadata_bytes() == 2*SIZE);
    test = (int*)srealloc(test,1000);
    assert(temp != test);
    for(int i=0;i<25;i++)
        assert(test[i] == i);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 1100);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 100);
    assert(_num_metadata_bytes() == 2*SIZE);
    sfree(test);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 1132);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 1132);
    assert(_num_metadata_bytes() == SIZE);
    int* ptr1 =(int*)smalloc(100);
    int* ptr2 =(int*)smalloc(100);
    int* ptr3 =(int*)smalloc(100);
    sfree(ptr1);
    sfree(ptr3);
    sfree(ptr2);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 1132);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 1132);
    assert(_num_metadata_bytes() == SIZE);
    return 0;
}
