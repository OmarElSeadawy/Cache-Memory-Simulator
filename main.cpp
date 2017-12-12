//#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <math.h>
#include <time.h>

using namespace std;

#define        ways            2
#define        DBG                1
#define        DRAM_SIZE        (64*1024*1024)
#define        CACHE_SIZE        (64*1024)
#define        blocksize        (32)
#define        numofblocks        ((CACHE_SIZE/blocksize)/ways)
#define        replacement       0		//0 random, 1 lru, 2 lfu, default fifo



enum cacheResType { MISS = 0, HIT = 1 };

unsigned int m_w = 0xF1230102;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x12344127;    /* must not be zero, nor 0x9068ffff */

struct cache_entry
{
	unsigned int tag;
	int valid;
	int recentuse;
	int frequse;
	int time2;
};

unsigned int rand_()
{
	m_z = 36969 * (m_z & 65535) + (m_z >> 16);
	m_w = 18000 * (m_w & 65535) + (m_w >> 16);
	return (m_z << 16) + m_w;  /* 32-bit result */
}

unsigned int memGen1()
{
	static unsigned int addr = 0;
	return (addr++) % (DRAM_SIZE);
}

unsigned int memGen2()
{
	static unsigned int addr = 0;
	return  rand_() % (128 * 1024);

}

unsigned int memGen3()
{
	return rand_() % (DRAM_SIZE);
}

unsigned int memGen4()
{
	static unsigned int addr = 0;
	return (addr++) % (1024);
}

unsigned int memGen5()
{
	static unsigned int addr = 0;
	return (addr++) % (1024 * 64);
}

unsigned int memGen6()
{
	static unsigned int addr = 0;
	return (addr += 256) % (DRAM_SIZE);
}

void getMin(cache_entry cache[][ways], int &mindex2, int x)
{
	int min = 999999999;
	for (int i = 0; i < ways; i++)
	{
		switch (x)
		{
		case 0:
			if (cache[0][i].time2 < min)
			{
				min = cache[0][i].time2;
				mindex2 = i;
			}
			break;
		case 1:
			if (cache[0][i].recentuse < min)
			{
				min = cache[0][i].recentuse;
				mindex2 = i;
			}
			break;
		case 2:
			if (cache[0][i].frequse < min)
			{
				min = cache[0][i].frequse;
				mindex2 = i;
			}
			break;
		}
	}

}


// Cache Simulator
cacheResType cacheSim(unsigned int addr, cache_entry cache[][ways])
{
	unsigned int offset_bits = log2(blocksize);
	unsigned int index_bits = log2(numofblocks);
	unsigned int tag_bits = 32 - index_bits - offset_bits;
	unsigned int fullytag_bits = 32 - offset_bits;

	unsigned int index = ((addr >> offset_bits) & ((int(pow(2, index_bits)) - 1)));
	unsigned int fullytag = (addr >> offset_bits);
	unsigned int tag = ((addr >> (offset_bits + index_bits) & ((int(pow(2, tag_bits)) - 1))));

	//cout << offset_bits << " " << index_bits << " " << tag_bits << endl;


	int time2 = 0;
	int recuse = 0;
	int mindex2 = 0;
	int lop = 0;
	int random;
	srand(time(NULL));


	if (numofblocks == 1)
	{

		for (int j = 0; j < ways; j++)
		{
			if (cache[0][j].valid == 1)
			{
				if (cache[0][j].tag == fullytag)
				{
					cache[0][j].recentuse=recuse++;
					cache[0][j].frequse++;
					return HIT;
				}
			}
		}
		for (int n = 0; n < ways; n++)
		{
			if (cache[0][n].valid == 0)
			{
				cache[0][n].tag = fullytag;
				cache[0][n].time2 = time2++;
				cache[0][n].frequse = 1;
				cache[0][n].recentuse = recuse++;
				cache[0][n].valid = 1;

				return MISS;
			}

		}

		switch (replacement)
		{
		case 0://INSERT RANDOM REMOVAL POLICY HERE
			random = rand() % ways;
			cache[0][random].tag = fullytag;
			cache[0][random].time2 = time2++;
			cache[0][random].frequse = 1;
			cache[0][random].recentuse = recuse++;
			cache[0][random].valid = 1;
			break;
		case 1://lru
			getMin(cache, mindex2, 1);
			cache[0][mindex2].tag = fullytag;
			cache[0][mindex2].time2 = time2++;
			cache[0][mindex2].frequse = 1;
			cache[0][mindex2].recentuse = recuse++;
			cache[0][mindex2].valid = 1;
			break;
		case 2://lfu
			getMin(cache, mindex2, 2);
			cache[0][mindex2].tag = fullytag;
			cache[0][mindex2].time2 = time2++;
			cache[0][mindex2].frequse = 1;
			cache[0][mindex2].recentuse = recuse++;
			cache[0][mindex2].valid = 1;
			break;
		default:
			//fifo
			getMin(cache, mindex2, 0);
			cache[0][mindex2].tag = fullytag;
			cache[0][mindex2].time2 = time2++;
			cache[0][mindex2].frequse = 1;
			cache[0][mindex2].recentuse = recuse++;
			cache[0][mindex2].valid = 1;
			break;
		}
		return MISS;
	}
	else
	{
		for (int i = 0; i < ways; i++)
		{
			if (cache[index][i].tag == tag)
			{
				cache[index][i].recentuse = recuse++;
				cache[index][i].frequse++;
				return HIT;
			}
		}
		if (ways == 1)
		{
			cache[index][0].tag = tag;
			cache[index][0].time2 = time2++;
			cache[index][0].frequse = 1;
			cache[index][0].recentuse = recuse++;
			cache[index][0].valid = 1;
			return MISS;
		}
		else
		{
			for (int k = 0; k < ways; k++)
			{
				if (cache[index][k].valid == 0)
				{
					cache[index][k].tag = tag;
					cache[index][k].time2 = time2++;
					cache[index][k].frequse = 1;
					cache[index][k].recentuse = recuse++;
					cache[index][k].valid = 1;
					return MISS;

				}
			}
			random = rand() % ways;
			cache[index][random].tag = tag;
			cache[index][random].time2 = time2++;
			cache[index][random].frequse = 1;
			cache[index][random].recentuse = recuse++;
			cache[index][random].valid = 1;
			return MISS;

			//INSERT RANDOM REMOVAL POLICY HERE
		}
	}
}

char *msg[2] = { "Miss", "Hit" };

int main()
{
	int inst = 0;
	float hitcount1 = 0, misscount1 = 0;
	float hitcount2 = 0, misscount2 = 0;
	float hitcount3 = 0, misscount3 = 0;
	float hitcount4 = 0, misscount4 = 0;
	float hitcount5 = 0, misscount5 = 0;
	float hitcount6 = 0, misscount6 = 0;
	cacheResType r1, r2, r3, r4, r5, r6;
	float missrate1, missrate2, missrate3, missrate4, missrate5, missrate6;

	cache_entry cache1[numofblocks][ways];
	cache_entry cache2[numofblocks][ways];
	cache_entry cache3[numofblocks][ways];
	cache_entry cache4[numofblocks][ways];
	cache_entry cache5[numofblocks][ways];
	cache_entry cache6[numofblocks][ways];

	for (int i = 0; i < numofblocks; i++)
	for (int j = 0; j < ways; j++)
	{
		cache1[i][j].valid = 0;
		cache2[i][j].valid = 0;
		cache3[i][j].valid = 0;
		cache4[i][j].valid = 0;
		cache5[i][j].valid = 0;
		cache6[i][j].valid = 0;

	}
	unsigned int addr1, addr2, addr3, addr4, addr5, addr6;
	cout << "Cache Simulator\n";
	// change the number of iterations into 10,000,000
	for (; inst<1000000; inst++)
	{
		addr1 = memGen1();
		addr2 = memGen2();
		addr3 = memGen3();
		addr4 = memGen4();
		addr5 = memGen5();
		addr6 = memGen6();
		r1 = cacheSim(addr1, cache1);
		r2 = cacheSim(addr2, cache2);
		r3 = cacheSim(addr3, cache3);
		r4 = cacheSim(addr4, cache4);
		r5 = cacheSim(addr5, cache5);
		r6 = cacheSim(addr6, cache6);

		if (r1)
			hitcount1++;
		else
			misscount1++;

		//cout << "0x" << setfill('0') << setw(8) << hex << addr1 << " (" << msg[r1] << ")\n";

		if (r2)
			hitcount2++;
		else
			misscount2++;

		//cout << "0x" << setfill('0') << setw(8) << hex << addr2 << " (" << msg[r2] << ")\n";

		if (r3)
			hitcount3++;
		else
			misscount3++;

		//cout << "0x" << setfill('0') << setw(8) << hex << addr3 << " (" << msg[r3] << ")\n";

		if (r4)
			hitcount4++;
		else
			misscount4++;

		//cout << "0x" << setfill('0') << setw(8) << hex << addr4 << " (" << msg[r4] << ")\n";

		if (r5)
			hitcount5++;
		else
			misscount5++;


	//	cout << "0x" << setfill('0') << setw(8) << hex << addr5 << " (" << msg[r5] << ")\n";

		if (r6)
		{
			hitcount6++;
			cout << hitcount6 <<" " << hex << addr6 << '\n';
		}
		else
			misscount6++;

		//cout << "0x" << setfill('0') << setw(8) << hex << addr6 << " (" << msg[r6] << ")\n";
	}

	missrate1 = misscount1 / (hitcount1 + misscount1);
	cout << "Miss Rate 1 :  " << missrate1 << "\n";

	missrate2 = misscount2 / (hitcount2 + misscount2);
	cout << "Miss Rate 2 : " << missrate2 << "\n";

	missrate3 = misscount3 / (hitcount3 + misscount3);
	cout << "Miss Rate 3 : " << missrate3 << "\n";

	missrate4 = misscount4 / (hitcount4 + misscount4);
	cout << "Miss Rate 4 : " << missrate4 << "\n";

	missrate5 = misscount5 / (hitcount5 + misscount5);
	cout << "Miss Rate 5 : " << missrate5 << "\n";

	missrate6 = misscount6 / (hitcount6 + misscount6);
	cout << "Miss Rate 6 : " << missrate6 << "\n";


	system("pause");
}