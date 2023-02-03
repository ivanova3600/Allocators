#include <iostream>
#include <ctime>
#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>

struct Chunk {
    void *block;
    Chunk *next;
};

class Allocator{
private:
unsigned char* used_blocks;
int size__[4];
int block_count;
size_t allocator_size;
Chunk** chunks;

void *find_block(int i){
    int j = 1;
    if(block_count){
        if(!chunks[i]->next){
            chunks[i]->next = (Chunk*)(std::malloc(sizeof(Chunk)));
            chunks[i]->next->block = chunks[i]->block;
            chunks[i]->next->next = nullptr;
            return chunks[i]->next->block;
        }
        Chunk *b = chunks[i]->next;
        Chunk *a = chunks[i]->next->next;
        while(a) {
            ++j; a = a->next;
            b = b->next;
        }
        if(j <= block_count){
            a = (Chunk*)(std::malloc(sizeof(Chunk)));
            a->block = chunks[i]->block + size__[i] * j;
            a->next = nullptr;
            b->next = a;
            return a->block;
        }
    }
    std::cout << "Can't allocate" << std::endl;
    return nullptr; 
}

void return_block(void *ptr, int i){
    if((void*)chunks[i]->next->block == ptr){
        Chunk *b = chunks[i];
        Chunk *c = chunks[i]->next;
        if(chunks[i]->next->next) {
            b->next = c->next;
            free(c); c = nullptr;
        }
        else {
            free(c); c = nullptr;
            b->next = c;
        }
        return;
    }
    Chunk *b = chunks[i]->next;
    Chunk *a = chunks[i]->next->next;  
    while(a){
        if(a->block == ptr){
            Chunk *c = a->next;
            if(a->next) b->next = c;
            else b->next = nullptr;
            free(a); a = nullptr;
            return;
        }
        a = a->next; b = b->next;
    }
    std::cout << "Can't deallocate" << std::endl;
}

void return_length(){
    block_count = 0;
    while(true){
        if(size__[0]*block_count + size__[1]*block_count + size__[2]*block_count + size__[3]*block_count <= allocator_size)
            block_count++;
        else {
            block_count--; break; 
        }
    }
}

public:
    Allocator(size_t size) : allocator_size(size) {
        size__[0] = 32; size__[1] = 64; size__[2] = 128; size__[3] = 256;
        used_blocks = (unsigned char*)(std::malloc(allocator_size)); 
        chunks = (Chunk**)(std::malloc(sizeof(Chunk*)*4));
        return_length();
        for(int i = 0; i < 4; ++i){
            chunks[i] = (Chunk*)(std::malloc(sizeof(Chunk)));
            if(!i) chunks[i]->block = used_blocks;
            else chunks[i]->block = (char*)chunks[i-1]->block + size__[i-1] * block_count;
        } 
    }

    void *allocate(size_t size){
        if(size <= 32) return find_block(0);
        else if(size <= 64) return find_block(1);
        else if(size <= 128) return find_block(2);
        else if(size <= 256) return find_block(3);
        
        std::cout << "Can't allocate" << std::endl;
        return nullptr;
        
    }

    void deallocate(void *ptr){
        if(ptr >= chunks[0]->block && ptr < chunks[1]->block) return_block(ptr, 0);
        else if(ptr >= chunks[1]->block && ptr < chunks[2]->block) return_block(ptr, 1);
        else if(ptr >= chunks[2]->block && ptr < chunks[3]->block) return_block(ptr, 2);
        else return_block(ptr, 3);       
    }

    void print(){
        std::cout << std::endl;
        for(int i = 0; i < 4; ++i){
            std::cout << size__[i] << ' ' << chunks[i]->block << std::endl;
            Chunk *a = chunks[i]->next;
            while(a){
                std::cout << a->block << std::endl;
                a = a->next;
            }
        }
    }

    ~Allocator(){
        for(int i = 0; i < 4; ++i){
            Chunk *a = chunks[i]->next;
            while(a){
                Chunk *b = a; a = a->next;
                free(b); b = nullptr;
            }
        }
        free(used_blocks);
    }
};

size_t random_from_range(int min, int max){
    return (rand() % (max - min + 1)) + min;
}

size_t random_number() {
    int MIN = 1;
    int MAX = 200;
    
    int temp = random_from_range(MIN, MAX);
    bool stop = random_from_range(0, 1);
    if (stop) {
        return temp;
    } else if (temp == MIN) {
        return MIN;
    } else {
        return random_from_range(MIN, temp - 1);
    }
}

int main()
{
    Allocator allocator(90000);

    std::vector<std::pair<char*,size_t>> blocks;
    std::vector<int> v;
    clock_t time;
    double end;
    
    time = clock();
    for(int i = 0; i < 500; ++i)  {
        size_t k = random_number();
        char *ptr = (char*)allocator.allocate(k);
        if(ptr) blocks.push_back(std::make_pair(ptr,k));    
    }
    end = double(clock() - time) / CLOCKS_PER_SEC;

    std::cout << "Geometr time: " << end <<std::endl;
    std::cout << "Size: " << blocks.size() <<std::endl;
    allocator.print();

    for(size_t i = 0;i < 500; i++) v.push_back(i);
    random_shuffle(v.begin(), v.end());

    time = clock();
    for(size_t i = 0; i < 500; ++i)
        allocator.deallocate(blocks[v[i]].first);
    end = double(clock() - time) / CLOCKS_PER_SEC;
    std::cout <<  "Geometr time: " << end <<std::endl;

    blocks.clear();

    time = clock();
    for(int i = 0; i < 500; ++i)  {
        size_t k = random_from_range(1, 200);
        char *ptr = (char*)allocator.allocate(k);
        if(ptr) blocks.push_back(std::make_pair(ptr,k));    
    }
    end = double(clock() - time) / CLOCKS_PER_SEC;

    std::cout << "Random time: " << end <<std::endl;
    std::cout << "Size: " << blocks.size() <<std::endl;

    time = clock();
    for(size_t i = 0; i < blocks.size(); ++i){
        allocator.deallocate(blocks[v[i]].first);
    }
    end = double(clock() - time) / CLOCKS_PER_SEC;
    std::cout <<  "Random time: " << end <<std::endl; 
/*
    Allocator al(10);
    al.print();
*/
    return 0;

}
/*Allocator allocator(500);

    std::vector<std::pair<char*,size_t>> blocks;
    std::vector<int> v;
    clock_t time;
    double end;
    
    time = clock();
    for(int i = 0; i < 500; ++i)  {
        size_t k = random_number();
        char *ptr = (char*)allocator.allocate(k);
        if(ptr)
            blocks.push_back(std::make_pair(ptr,k));    
    }
    end = double(clock() - time) / CLOCKS_PER_SEC;

    std::cout << "Time: " << end <<std::endl;
    std::cout << "Size: " << blocks.size() <<std::endl;

    for(size_t i = 0;i < blocks.size(); i++) v.push_back(i);
    random_shuffle(v.begin(), v.end());

    time = clock();
    for(size_t i = 0; i < blocks.size(); ++i){
        allocator.deallocate(blocks[v[i]].first);
    }
    end = double(clock() - time) / CLOCKS_PER_SEC;
    std::cout <<  "time: " << end <<std::endl;

    blocks.clear();

    time = clock();
    for(int i = 0; i < 500; ++i)  {
        size_t k = random_from_range(1, 200);
        char *ptr = (char*)allocator.allocate(k);
        if(ptr)
            blocks.push_back(std::make_pair(ptr,k));    
    }
    end = double(clock() - time) / CLOCKS_PER_SEC;

    std::cout << "Time: " << end <<std::endl;
    std::cout << "Size: " << blocks.size() <<std::endl;

    time = clock();
    for(size_t i = 0; i < blocks.size(); ++i){
        allocator.deallocate(blocks[v[i]].first);
    }
    end = double(clock() - time) / CLOCKS_PER_SEC;
    std::cout <<  "time: " << end <<std::endl; 
    */