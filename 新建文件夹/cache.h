#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
#include<vector>
using namespace std;

#define L1_CACHE_SETS 16
#define L2_CACHE_SETS 16
#define L2_CACHE_WAYS 8
#define MEM_SIZE 4096
#define BLOCK_SIZE 4 // bytes per block
#define DM 0
#define SA 1

struct cacheBlock
{
	int tag; // you need to compute offset and index to find the tag.
	int lru_position; // for SA only
	int data; // the actual data stored in the cache/memory
	bool valid;
	// add more things here if needed
};

struct Stat
{
	int missL1; 
	int missL2; 
	int accL1;
	int accL2;
	// add more stat if needed. Don't forget to initialize!
};

class cache {
private:
	cacheBlock L1[L1_CACHE_SETS]; // 1 set per row.
	cacheBlock L2[L2_CACHE_SETS][L2_CACHE_WAYS]; // x ways per row 
	Stat myStat;
	// add more things here
public:
	cache();
	bool controller(bool MemR, bool MemW, int* data, int adr, int* myMem);
	string de2bi(int adr);
	int bi2de(string str);

	bool checkadrL1(string adr);

	int checkadrL2(string adr);

	void Lru(int index,int way);

	void removeLru(int index,int way);

	void write2cacheL2(int index,int tag,int data);

	vector<float> return_result();
	// add more functions here ...	
};


