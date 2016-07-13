#ifndef _AMAP_H_
#define _AMAP_H_

#include <string.h>


namespace MRPC {
    template <class T>
    class amap {
    public:
        struct element {
            char key[64];
            T value;
        };
        amap() {
            size = 10;
            count = 0;
            elements = (struct element*)malloc(sizeof(struct element) * 10);
        }
        struct element* elements;
        void set(const char* key, T value) {
            if(count == size) {
                elements = (struct element*)realloc(elements, size * 2);
                size = size * 2;
            }
            T* current = get(key);
            if(current == NULL) {
                strncpy(elements[count].key, key, 64);
                current = &elements[count].value;
            }
            *current = value;
        }
        T* get(const char* key) {
            for(int i = 0; i < count; i++) {
                if(strcmp(key, elements[i].key) == 0)
                    return &elements[i].value;
            }
            return NULL;
        };
        int count;
    private:
        int size;
    };

}

#endif