#include <unistd.h>
#define MAXIMUM_SIZE 100000000

void* smalloc(size_t size){
    if (0 == size || size > MAXIMUM_SIZE)
        return NULL;
    void* address = sbrk(size);
    if (address == (void*)-1)
        return NULL;
    return address;
}
