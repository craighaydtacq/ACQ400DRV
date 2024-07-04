#include <stdio.h>
#include <stdlib.h>
#include <string.h>


template <class T>
int ramp(int nsam, int step, int ssize, T start=0)
{
	unsigned long long nwords = nsam;
	nwords *= ssize;
	T xx = start;
	int ch = 0;

	fprintf(stderr, "ramp<%lu> nwords:%llu MB:%llu \n", 
			sizeof(T), nwords, (nwords*sizeof(T))/0x100000);

	for (; nwords; nwords--){
		fwrite(&xx, sizeof(xx), 1, stdout);
		if (++ch >= ssize){
			xx += step;
			ch = 0;
		}
	}
	return 0;
}

template <class T>
int ramp_split(int nsam, int step, int ssize, T start=0)
{
	int nwords = nsam * ssize;
	int nbits = sizeof(T)*8;
	T xx = start;
	int ch = 0;
	for (; nwords; nwords--){
		T yy = 0;
		for (int xbit = 0, ybit = 0; xbit < nbits/2; xbit++, ybit +=2){
			if (xx & 1<<xbit){
				yy |= 1<<ybit;
			}
		}
		fwrite(&yy, sizeof(xx), 1, stdout);
		if (++ch >= ssize){
			xx += step;
			ch = 0;
		}
	}
	return 0;
}
int main(int argc, char* argv[])
{
	if (argc == 1 || strstr(argv[1], "h") != 0){
		printf("ramp [nsam=1024 [step=1 [wsize=4 [ssize=1]]]\n");
		printf("eg ramp 64000 1 4 32\n");
		exit(1);
	}
	int length = argc>1? strtoul(argv[1], 0, 0): 1024;
	int step = argc>2? atoi(argv[2]): 1;
	int wsize = argc>3?  atoi(argv[3]): 4;
	int ssize = argc>4?  atoi(argv[4]): 1;
	int splitbits = 0;
	unsigned long id = 0;
	unsigned long long ullid;
	bool id_set = 0;
	if (getenv("RAMP_ID")){
		id = strtoul(getenv("RAMP_ID"), 0, 0);
		id_set = 1;
	}
	if (getenv("SPLITBITS")){
		splitbits = atoi(getenv("SPLITBITS"));
	}


	if (splitbits == 0){
		switch(wsize){
		case 2:
			return ramp<short>(length, step, ssize);
		case 4:
			return ramp<int>(length, step, ssize);
		case 8:
			if (!id_set){
				id = 0x6400;
			}
			ullid = id; ullid <<= 48;		
			return ramp<unsigned long long>(length, step, ssize, ullid);
		default:
			fprintf(stderr, "ERROR: only size 4 supported\n");
			return -1;
		}
	}else{
		switch(wsize){
		case 2:
			return ramp_split<short>(length, step, ssize);
		case 4:
			return ramp_split<int>(length, step, ssize);
		case 8:
                       if (!id_set){
                                id = 0x6400;
                        }
                        ullid = id; ullid <<= 48;			
			return ramp_split<unsigned long long>(length, step, ssize, ullid);
		default:
			fprintf(stderr, "ERROR: only size 4 supported\n");
			return -1;
		}
	}
}
