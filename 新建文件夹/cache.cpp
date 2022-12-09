#include "cache.h"

cache::cache()
{
	for (int i=0; i<L1_CACHE_SETS; i++)
		L1[i].valid = false; 
	for (int i=0; i<L2_CACHE_SETS; i++)
		for (int j=0; j<L2_CACHE_WAYS; j++)
			L2[i][j].valid = false; 

	this->myStat.missL1 =0;
	this->myStat.missL2 =0;
	this->myStat.accL1 =0;
	this->myStat.accL2 =0;
}

string cache::de2bi(int adr) {
	string result = "";
	//cout << "" << endl;
	//cout << adr << endl;

	while (adr>=1) {
		int temp_int = adr % 2;
		
		result =to_string(temp_int) + result;
		
		adr = adr / 2;
	}
	while (result.size()<12) {
		result = to_string(0) + result;
	}
	return result;
}

int cache::bi2de(string str) {
	int result = 0;
	//cout << "" << endl;

	//cout << str << endl;
	for (int i = 0; i < str.size();i++) {
		result += (str[i]-'0')*pow(2,i);
	}
	//cout << result << endl;
	//cout << "" << endl;
	return result;
}

bool cache::checkadrL1(string adr){
	int adr_index = bi2de(adr.substr(0,4));
	int adr_tag = bi2de(adr.substr(4,12));
	return L1[adr_index].valid&&(L1[adr_index].tag==adr_tag);
}

int cache::checkadrL2(string adr) {
	int adr_index = bi2de(adr.substr(0, 4));
	
	int adr_tag = bi2de(adr.substr(4, 12));
	for (int i = 0; i < 8; i++) {
		if (L2[adr_index][i].valid && (L2[adr_index][i].tag == adr_tag)) {
			return i;
		}
	}
	return -1;
}

void cache::Lru(int index,int way) {
	int lrumax = -1;
	for (int i = 0; i < 8; i++) {
		if (L2[index][i].valid&&(L2[index][i].lru_position > lrumax)) {
			lrumax = L2[index][i].lru_position;
		}
	}

	if (L2[index][way].valid==false) {
		
		L2[index][way].lru_position = lrumax + 1;
	}
	else {
		int old_lru = L2[index][way].lru_position;
		for (int i = 0; i < 8;i++) {
			if(L2[index][i].valid && (L2[index][i].lru_position>old_lru)){
				L2[index][i].lru_position--;
			}
		}
		L2[index][way].lru_position = lrumax;
	
	}
}

void cache::removeLru(int index,int way) {
	int old_lru = L2[index][way].lru_position;
	for (int i = 0; i < 8;i++) {
		if (L2[index][i].lru_position>old_lru) {
			L2[index][i].lru_position--;
		}
	}
}

void cache::write2cacheL2(int index,int tag,int data) {
	int way = 0;
	int mini = INT_MAX;
	for (int i = 0; i < 8;i++) {
		if (L2[index][i].valid==true) {//search the valid data
			if (L2[index][i].lru_position<mini) {//record the oldest position
				way = i;
				mini = L2[index][i].lru_position;
			}
		}
		else {//find a invalid data 
			way = i;
			break;
		}
	}
	L2[index][way].data = data;
	L2[index][way].tag = tag;
	L2[index][way].valid = true;
	Lru(index,way);
}

bool cache::controller(bool MemR, bool MemW, int* data, int adr, int* myMem)
{
	string real_adr=de2bi(adr);
	reverse(real_adr.begin(),real_adr.end());
	int adr_index = bi2de(real_adr.substr(0, 4));
	int adr_tag = bi2de(real_adr.substr(4, 12));
	//cout << adr_index << endl;
	//cout << adr_tag << endl;
	int L2_token = checkadrL2(real_adr);
	if (MemR) {
		if (!checkadrL1(real_adr)) {//not in the L1
			if (L2_token == -1) {//not in the L2 as well
				if (L1[adr_index].valid) {
					write2cacheL2(adr_index,L1[adr_index].tag,L1[adr_index].data);
				}
				//
				L1[adr_index].data = myMem[adr];
				L1[adr_index].valid = true;
				L1[adr_index].tag = adr_tag;
				
				this->myStat.missL2++;
				//cout << "L1miss " << "L2miss" << endl;
			}
			else {//not in L1, in L2
				if (L1[adr_index].valid) {//have data inside L1
				int L1exdata = L1[adr_index].data;
				int L1extag = L1[adr_index].tag;
				L1[adr_index].data = L2[adr_index][L2_token].data;
				L1[adr_index].tag = L2[adr_index][L2_token].tag;
				L2[adr_index][L2_token].data = L1exdata;
				L2[adr_index][L2_token].tag = L1extag;
				Lru(adr_index, L2_token);//put in L2
				}
				else {//L1 dont have any data
					L1[adr_index].data = L2[adr_index][L2_token].data;
					L1[adr_index].tag = L2[adr_index][L2_token].tag;
					L1[adr_index].valid = true;
					L2[adr_index][L2_token].data = 0;//remove from L2
					L2[adr_index][L2_token].tag = -1;
					L2[adr_index][L2_token].valid = false;
					removeLru(adr_index,L2_token);
				}
				this->myStat.accL2++;
				//cout << "L1miss " << "L2hit" << endl;
			}
			this->myStat.missL1++;
		}
		else {//in the L1
			this->myStat.accL1++;
			//cout << "L1hit" << endl;
			
		}
	}
	if (MemW) {
		if (!checkadrL1(real_adr)&&L2_token==-1) {//not in both
			//if (L1[adr_index].valid) {//L1 have data inside
			//	int temp_data = L1[adr_index].data;
			//	int temp_tag = L1[adr_index].tag;
			//	L1[adr_index].data = *data;
			//	L1[adr_index].tag =adr_tag;
			//	write2cacheL2(adr_index, temp_tag, temp_data);
			//}
			//else {
			//	L1[adr_index].data = *data;
			//	L1[adr_index].tag = adr_tag;
			//	L1[adr_index].valid = true;
				
			//}
			this->myStat.missL1++;
			this->myStat.missL2++;
			
		}
		else if (!checkadrL1(real_adr)&&L2_token!=-1) {//in L2 not in L1
			if (L1[adr_index].valid) {//L1 have data inside
				int temp_data = L1[adr_index].data;
				int temp_tag = L1[adr_index].tag;
				L1[adr_index].data = L2[adr_index][L2_token].data;
				L1[adr_index].tag = L2[adr_index][L2_token].tag;
				write2cacheL2(adr_index,temp_tag,temp_data);				
			}
			else {//L1 dont have data inside
				L1[adr_index].data = L2[adr_index][L2_token].data;
				L1[adr_index].tag = L2[adr_index][L2_token].tag;
				L1[adr_index].valid = true;
				L2[adr_index][L2_token].data =0;
				L2[adr_index][L2_token].tag =-1;
				L2[adr_index][L2_token].valid = false;
				removeLru(adr_index, L2_token);

			}
			this->myStat.missL1++;
			this->myStat.accL2++;
		}
		else if(checkadrL1(real_adr)){
			this->myStat.accL1++;
		}
	
		myMem[adr] = *data;//write into the main memory
	}
	// add your code here

	return true;
}

vector<float> cache::return_result() {
	vector<float> result;
	result.push_back(this->myStat.missL1);
	result.push_back(this->myStat.accL1);
	result.push_back(this->myStat.missL2);
	result.push_back(this->myStat.accL2);
	//cout << this->myStat.missL1 << endl;
	//cout << this->myStat.accL1 << endl;
	//cout << this->myStat.missL2 << endl;
	//cout << this->myStat.accL2 << endl;

	return result;
}