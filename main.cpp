#include <iostream>
#include "assert.h"
#include "malloc.h"
const size_t SIZE=_size_meta_data();
int main() {
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
    test = (int*)scalloc(200,4);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 1200);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 400);
    assert(_num_metadata_bytes() == 2*SIZE);
    for(int i=0;i<200;i++)
        assert(test[i] == 0);
    sfree(test);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 1200);
    assert(_num_free_blocks() == 2);
    assert(_num_free_bytes() == 1200);
    assert(_num_metadata_bytes() == 2*SIZE);
    test = (int*)smalloc(800);
    int* temp = test;
    test = (int*)srealloc(test,100);
    assert(temp == test);
    for(int i =0;i< 100;i++)
        test[i]=i;
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 1200);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 400);
    assert(_num_metadata_bytes() == 2*SIZE);
    test = (int*)srealloc(test,1000);
    assert(temp != test);
    for(int i=0;i<100;i++)
        assert(test[i] == i);
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 2200);
    assert(_num_free_blocks() == 2);
    assert(_num_free_bytes() == 1200);
    assert(_num_metadata_bytes() == 3*SIZE);
    sfree(test);
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 2200);
    assert(_num_free_blocks() == 3);
    assert(_num_free_bytes() == 2200);
    assert(_num_metadata_bytes() == 3*SIZE);
    return 0;
}
