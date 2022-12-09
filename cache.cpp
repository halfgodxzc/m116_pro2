#include "cache.h"

cache::cache()
{
	for (int i=0; i<L1_CACHE_SETS; i++)
		L1[i].valid = false; 
	for (int i = 0; i < V_SETS; i++)
	{
		V[i].valid = false;
		V[i].lru_position = 0;
	}
	for (int i=0; i<L2_CACHE_SETS; i++)
		for (int j = 0; j < L2_CACHE_WAYS; j++)
		{
			L2[i][j].valid = false;
			L2[i][j].lru_position = 0;

		}

	this->myStat.missL1 =0;
	this->myStat.missL2 =0;
	this->myStat.accL1 =0;
	this->myStat.accL2 =0;
	this->myStat.accV = 0;
	this->myStat.missV = 0;
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

string cache::de2bi4(int adr) {
	string result = "";
	//cout << "" << endl;
	//cout << adr << endl;

	while (adr >= 1) {
		int temp_int = adr % 2;

		result = to_string(temp_int) + result;

		adr = adr / 2;
	}
	while (result.size() < 4) {
		result = to_string(0) + result;
	}
	
	return result;
}

string cache::de2bi8(int adr) {
	string result = "";
	//cout << "" << endl;
	//cout << adr << endl;

	while (adr >= 1) {
		int temp_int = adr % 2;

		result = to_string(temp_int) + result;

		adr = adr / 2;
	}
	while (result.size() < 8) {
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

int cache::checkadrV(string adr) {
	int adr_v = bi2de(adr.substr(0,12));
	for (int i = 0; i < 4;i++) {
		if (V[i].valid&&V[i].tag==adr_v) {
			return i;
		}
	}
	return -1;
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

void cache::LruV(int way) {
	int lrumax = 0;
	for (int i = 0; i < 4; i++) {
		if (V[i].valid && (V[i].lru_position > lrumax)) {
			lrumax = V[i].lru_position;
		}
	}

	if ( V[way].valid == false) {

		V[way].lru_position = lrumax + 1;
	}
	else {
		int old_lru = V[way].lru_position;
		for (int i = 0; i < 4; i++) {
			if (V[i].valid && (V[i].lru_position > old_lru)) {
			V[i].lru_position--;
			}
		}
		V[way].lru_position = lrumax;


	}
}

int cache::checkV() {
	int mini = INT_MAX;
	int way = -1;
	for (int i = 0; i < 4;i++) {
		if (V[i].valid&&(mini>V[i].lru_position)) {
			mini = V[i].lru_position;
			way = i;
		}
		if (V[i].valid==false) {
			return i;
		}
	}
	return way;
}

void cache::Lru(int index,int way) {
	int lrumax = 0;
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

void cache::removeLruV(int way) {
	int old_lru = V[way].lru_position;
	for (int i = 0; i < 4; i++) {
		if (V[i].lru_position > old_lru) {
			V[i].lru_position -= 1;
		}

		V[way].valid = false;
	}
}

void cache::removeLru(int index,int way) {
	int old_lru = L2[index][way].lru_position;
	for (int i = 0; i < 8;i++) {
		if (L2[index][i].lru_position>old_lru) {
			L2[index][i].lru_position -= 1;
		}
	
		L2[index][way].valid = false;
	}
}

void cache::write2cacheV(int adr, int data) {
	int way = 0;
	int mini = INT_MAX;
	for (int i = 0; i < 4; i++) {
		if (V[i].valid == true) {//search the valid data
			if (V[i].lru_position < mini) {//record the oldest position
				way = i;
				mini = V[i].lru_position;
			}
		}
		else {//find a invalid data 
			way = i;
			break;
		}
		LruV(way);
		V[way].data = data;
		V[way].tag = adr;
		V[way].valid = true;
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
	Lru(index,way);
	L2[index][way].data = data;
	L2[index][way].tag = tag;
	L2[index][way].valid = true;

	//cout << "---" << "way" << "---" << endl;
	//cout << way << endl;
	//cout << "---" << "way" << "---" << endl;

	
}

bool cache::controller(bool MemR, bool MemW, int* data, int adr, int* myMem)
{
	string real_adr=de2bi(adr);
	reverse(real_adr.begin(),real_adr.end());
	int adr_index = bi2de(real_adr.substr(0, 4));
	int adr_tag = bi2de(real_adr.substr(4, 12));
	int v_adr_tag = bi2de(real_adr.substr(0,12));
	int L2_token = checkadrL2(real_adr);
	int V_token = checkadrV(real_adr);
	//cout << adr_index<<" "<<adr_tag <<" "<<L2_token<< endl;
	
	if (MemR) {
		if (!checkadrL1(real_adr)) {//not in the L1
			

			if (V_token==-1&& L2_token == -1) {//not in the L2 and vit as well
				if (L1[adr_index].valid) {
					int V_index = checkV();
					
					string adr_v_s = ( de2bi8(L1[adr_index].tag)+de2bi4(adr_index) );
					reverse(adr_v_s.begin(), adr_v_s.end());
					int adr_v = bi2de(adr_v_s);
					int V_data = V[V_index].data;
					int V_tag = V[V_index].tag / 16;
					string V_adr = de2bi(V[V_index].tag);
					reverse(V_adr.begin(),V_adr.end());
					int L2index = bi2de(V_adr.substr(0,4));
					LruV(V_index);
					V[V_index].tag = adr_v;
					V[V_index].data = L1[adr_index].data;
					V[V_index].valid = true;
					//write2cacheV(adr_v,L1[adr_index].data);
					write2cacheL2(L2index,V_tag,V_data);
				}
				//
				L1[adr_index].data = myMem[adr];
				L1[adr_index].valid = true;
				L1[adr_index].tag = adr_tag;
				
				this->myStat.missL2++;
				this->myStat.missV++;
				//cout << "L1miss " << "L2miss" << endl;
			} 
			else if (V_token!=-1) {//in the vit
				if (L1[adr_index].valid) {
					int L1data = L1[adr_index].data;
					int L1tag = L1[adr_index].tag;
					string adr_v_s = (  de2bi8(L1tag)+de2bi4(adr_index));
					reverse(adr_v_s.begin(), adr_v_s.end());
					int V_adr = bi2de(adr_v_s);	
					L1[adr_index].data = V[V_token].data;
					L1[adr_index].tag = V[V_token].tag / 16;
					int V_index = checkV();
					LruV(V_index);
					V[V_index].tag = V_adr;
					V[V_index].data = L1[adr_index].data;
					V[V_index].valid = true;
					//write2cacheV(V_adr,L1data);
					this->myStat.accV++;
				}
				else {
					L1[adr_index].data = V[V_token].data;
					L1[adr_index].tag = V[V_token].tag / 16;
					L1[adr_index].valid = true;
					V[V_token].valid = false;
					removeLruV(V_token);
					this->myStat.accV++;

				}
			}
			else if(L2_token!=-1){//not in L1, in L2
				if (L1[adr_index].valid) {//have data inside L1
					int V_index = checkV();
					string adr_v_s = ( de2bi8(L1[adr_index].tag)+de2bi4(adr_index) );
					reverse(adr_v_s.begin(), adr_v_s.end());
					int adr_v = bi2de(adr_v_s);
					int V_data = V[V_index].data;
					int V_tag = V[V_index].tag / 16;
					string V_adr = de2bi(V[V_index].tag);
					reverse(V_adr.begin(), V_adr.end());
					int L2index = bi2de(V_adr.substr(0, 4));
					LruV(V_index);
					V[V_index].tag = adr_v;
					V[V_index].data = L1[adr_index].data;
					V[V_index].valid = true;
					//write2cacheV(adr_v, L1[adr_index].data);
					L1[adr_index].tag = L2[adr_index][L2_token].tag;
					L1[adr_index].data = L2[adr_index][L2_token].data;
					write2cacheL2(L2index, V_tag, V_data);
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
				this->myStat.missV++;
				//cout << "L1miss " << "L2hit" << endl;
			}

			this->myStat.missL1++;
		}
		else {//in the L1
			this->myStat.accL1++;
			
		}
	}
	if (MemW) {
		if (!checkadrL1(real_adr) && V_token == -1 && L2_token == -1) {//not in all
			
			this->myStat.missL1++;
			this->myStat.missL2++;
			this->myStat.missV++;

		}
		else if (!checkadrL1(real_adr) && V_token != -1) { //not in L1 but in Victim
			if (L1[adr_index].valid) {//L1 have data inside
				int temp_data = L1[adr_index].data;
				int temp_tag = L1[adr_index].tag;
				string adr_v_s = ( de2bi8(temp_tag)+ de2bi4(adr_index));
				reverse(adr_v_s.begin(),adr_v_s.end());
				int adr_v = bi2de(adr_v_s);
				int L1tag = V[V_token].tag / 16;
				L1[adr_index].data = *data;
				L1[adr_index].tag = L1tag;
				int V_index = checkV();
				LruV(V_index);
				V[V_index].tag = adr_v;
				V[V_index].data =temp_data;
				V[V_index].valid = true;
				//write2cacheV(adr_v, temp_data);
				

			}
			else {
				L1[adr_index].data = V[V_token].data;
				L1[adr_index].tag = V[V_token].tag / 16;
				L1[adr_index].valid = true;
				V[V_token].valid = false;
				removeLruV(V_token);
			}
				this->myStat.missL1++;
				this->myStat.accV++;
		}
		else if (!checkadrL1(real_adr) && V_token == -1 && L2_token != -1) {//in L2 not in L1
			if (L1[adr_index].valid) {//L1 have data inside
				int temp_data = L1[adr_index].data;
				int temp_tag = L1[adr_index].tag;
				string adr_v_s = ( de2bi8(temp_tag)+de2bi4(adr_index));
				reverse(adr_v_s.begin(),adr_v_s.end());
				int adr_v= bi2de(adr_v_s);
				L1[adr_index].data = L2[adr_index][L2_token].data;
				L1[adr_index].tag = L2[adr_index][L2_token].tag;
				int V_way = checkV();
				int V_data = V[V_way].data;
				int L2tag = V[V_way].tag / 16;
				LruV(V_way);
				V[V_way].tag = adr_v;
				V[V_way].data = temp_data;
				V[V_way].valid = true;
				//write2cacheV(adr_v,temp_data);
				int L2index = bi2de(adr_v_s.substr(0,4));
				write2cacheL2(L2index,L2tag,V_data);				
				
				this->myStat.missV++;
			}
			else {//L1 dont have data inside
				L1[adr_index].data = L2[adr_index][L2_token].data;
				L1[adr_index].tag = L2[adr_index][L2_token].tag;
				L1[adr_index].valid = true;
				L2[adr_index][L2_token].data =0;
				L2[adr_index][L2_token].tag =-1;
				L2[adr_index][L2_token].valid = false;
				removeLru(adr_index, L2_token);
			
				this->myStat.missV++;


			}
			this->myStat.missL1++;
			this->myStat.accL2++;
		}
		else if(checkadrL1(real_adr)){
			

			L1[adr_index].data = *data;
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
	result.push_back(this->myStat.accV);
	//cout << this->myStat.missL1 << endl;
	//cout << this->myStat.accL1 << endl;
	//cout << this->myStat.missL2 << endl;
	//cout << this->myStat.accL2 << endl;
	//for (int i = 0; i < 8;i++) {
		//cout << L2[0][i].lru_position << " ";
	//}
	//cout << endl;

	return result;
}