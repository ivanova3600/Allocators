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
size_t allocator_size;
size_t curr;
Chunk** chunks;
int size__[4];
size_t size_[4];
int block_count;

void *find_block(int k){
	if(chunks[k]->next){
		void *r = chunks[k]->next->block;
		Chunk *a = chunks[k]->next;
		Chunk *c = chunks[k]->next->next;
		chunks[k]->next = c; free(a); a = nullptr;
		curr += size__[k];
		size_[k] -= size__[k];
		return r;
	}
	std::cout << "Can't allocate" << std::endl;
	return nullptr;	
}

void return_block(void *ptr, int k){
	int i = 0;
	while(chunks[k]->block + size__[k] * i != ptr && i <= block_count) i++;
	if(i > block_count){
		std::cout << "Can't deallocate" << std::endl; 
		return;
	}
	if(chunks[k]->next > ptr){
		Chunk *b = chunks[k]->next;
		Chunk *a = chunks[k]->next->next;
		Chunk *c = (Chunk*)(std::malloc(sizeof(Chunk)));
		c->block = ptr; c->next = a;
		b->next = c;
		curr -= size__[k]; size_[k] += size__[k];
		return;
	}
	if(chunks[k]->next){
		Chunk *b = chunks[k]->next;
		Chunk *a = chunks[k]->next->next;
		while(a){
			if(b->block <= ptr && ptr + size__[k] >= a->block){
				Chunk *c = (Chunk*)(std::malloc(sizeof(Chunk)));
				c->block = ptr; c->next = a; b->next = c;
				curr -= size__[k]; size_[k] += size__[k];

				return;
			}
			a = a->next; b = b->next;
		}
		Chunk *c = (Chunk*)(std::malloc(sizeof(Chunk)));
		c->block = ptr; c->next = nullptr; b->next = c;
		curr -= size__[k]; size_[k] += size__[k];
		return;
	}
	chunks[k]->next = (Chunk*)(std::malloc(sizeof(Chunk)));
	chunks[k]->next->block = ptr; chunks[k]->next->next = nullptr;
	curr -= size__[k]; size_[k] += size__[k];

}

void return_length(){
    block_count = 0;
    while(true){
        if(size__[0]*block_count + size__[1]*block_count + size__[2]*block_count + size__[3]*block_count <= allocator_size)
            block_count++;
        else {
            --block_count; break; 
        }
    }
}
public:
	Allocator(size_t size) : allocator_size(size) {
		size__[0] = 32; size__[1] = 64; size__[2] = 128; size__[3] = 256;	
		return_length();
		used_blocks = (unsigned char*)(std::malloc(allocator_size)); 
		chunks = (Chunk**)(std::malloc(sizeof(Chunk*)*4));
		for(int i = 0; i < 4; ++i){
			size_[i] = block_count*size__[i];
			chunks[i] = (Chunk*)(std::malloc(sizeof(Chunk)));
			if(!i) chunks[i]->block = used_blocks;
			else chunks[i]->block = (char*)chunks[i-1]->block + size__[i-1] * block_count;				
			chunks[i]->next = (Chunk*)(std::malloc(sizeof(Chunk)));
			Chunk *a = chunks[i]->next;
			for(int j = 0; j < block_count; j++){
				if(!i) a->block = used_blocks + size__[i] * j;
				else a->block = (char*)chunks[i-1]->block + size__[i-1] * block_count + size__[i] * j; 
				a->next = (Chunk*)(std::malloc(sizeof(Chunk)));
				a = a->next;
			}
			a = nullptr;
		} 
		curr = 0;
	}

	void *allocate(size_t size){
		if(size <= 32) return find_block(0);
		else if(size <= 64) return find_block(1);
		else if(size <= 128) return find_block(2);
		else if(size <= 256) return find_block(3);
		else{
			std::cout << "Can't allocate" << std::endl;
			return nullptr;
		}
	}

	void deallocate(void *ptr){
		if(ptr >= chunks[0]->block && ptr < chunks[1]->block) return_block(ptr, 0);
	 	else if(ptr >= chunks[1]->block && ptr < chunks[2]->block) return_block(ptr, 1);
		else if(ptr >= chunks[2]->block && ptr < chunks[3]->block) return_block(ptr, 2);
		else if(ptr >= chunks[3]->block && ptr < (void*)used_blocks + allocator_size) return_block(ptr, 3);
		else std::cout << "Can't deallocate" << std::endl;
	}

	void print(){
	/*	for(int i = 0; i < 4; ++i){ 
			std::cout << i << ' ' << size_[i] << std::endl; 
			Chunk *a = chunks[i]->next;
			while(a){
				std::cout << a->block << std::endl;
				a = a->next;
			} 
		} */
		std::cout << curr << std::endl;
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

size_t rrandom(int min, int max){
	return (rand() % (max - min + 1)) + min;
}

size_t geom_random() {
    int min = 1, max = 200;
    int tmp = rrandom(min, max);
    int stop = rrandom(0, 1);
    if(stop) return tmp;
    else if (tmp == min) return min;
    else return rrandom(min, tmp - 1);
}

int main()
{
	Allocator allocator(100000);
	std::vector<std::pair<char*,size_t>> blocks;
	std::vector<int> v;
	clock_t time;
	double end;
//	size_t size = 0;
	
	time = clock();
	for(int i = 0; i < 500; ++i)  {
		size_t k = geom_random();
//		size += k;
		char *ptr = (char*)allocator.allocate(k);
		if(ptr) blocks.push_back(std::make_pair(ptr,k));	
	}
	end = double(clock() - time) / CLOCKS_PER_SEC;

	std::cout << "Geometr time: " << end <<std::endl;
	std::cout << "Size: " << blocks.size() << std::endl; 

	for(size_t i = 0;i < blocks.size(); i++) v.push_back(i);
    random_shuffle(v.begin(), v.end());

	time = clock();
	for(size_t i = 0; i < blocks.size(); ++i) allocator.deallocate(blocks[v[i]].first);
	end = double(clock() - time) / CLOCKS_PER_SEC;
	std::cout <<  "Geometr time: " << end <<std::endl;

	blocks.clear();
//	size = 0;

	time = clock();
	for(int i = 0; i < 500; ++i)  {
		size_t k = rrandom(1, 200); // size += k;
		char *ptr = (char*)allocator.allocate(k);
		if(ptr) blocks.push_back(std::make_pair(ptr,k));	
	}
	end = double(clock() - time) / CLOCKS_PER_SEC;

	std::cout << "Random time: " << end <<std::endl;
	std::cout << "Size: " << blocks.size() << std::endl;

	for(size_t i = 0;i < blocks.size(); i++) v.push_back(i);
    random_shuffle(v.begin(), v.end());

	time = clock();
	for(size_t i = 0; i < blocks.size(); ++i) allocator.deallocate(blocks[v[i]].first);
	
	end = double(clock() - time) / CLOCKS_PER_SEC;
	std::cout <<  "Random time: " << end <<std::endl; 
	return 0;

}