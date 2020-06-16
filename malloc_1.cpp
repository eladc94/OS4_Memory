#include <unistd.h>

void* smalloc(size_t size){
    if (0 == size || size > 100000000)
        return NULL;
    void* address = sbrk(size);
    if ((long)address == -1)
        return NULL;
    return address;
}
