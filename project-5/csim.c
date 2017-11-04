// Name: Hoang Phan
// LoginID: hoangpha

#include "cachelab.h"
#include "dogfault.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

typedef struct Line Line;
typedef struct Set Set;
typedef struct Cache Cache;

struct Line{
	int valid;
	long unsigned int tag;
	long unsigned int memory;
};

struct Set{
	Line* lines;
	int* policy;
	long int used;
	long int size;
};


struct Cache{
	Set* sets;
	long int used;
	long int size;
};

void initCache(Cache* c, long int initialSize) {
	c->sets = (Set*) malloc(initialSize * sizeof(Set));
	c->used = 0;
	c->size = initialSize;
}

void addSet(Cache* c, Set set) {
	if (c->used == c->size) {
		c->size *= 2;
		c->sets = (Set*) realloc(c->sets, c->size * sizeof(Set));
	}
	c->sets[c->used] = set;
	++c->used;
}

void initSet(Set* s, long int initialSize) {
	s->lines = (Line*) malloc(initialSize * sizeof(Line));
	s->policy = (int*) malloc(initialSize * sizeof(int));
	s->used = 0;
	s->size = initialSize;
}

void addLine(Set* s, Line line) {
	if (s->used == s->size) {
		s->size *= 2;
		s->lines = (Line*) realloc(s->lines, s->size * sizeof(Line));
		s->policy = (int*) realloc(s->policy, s->size * sizeof(int));
	}
	s->lines[s->used] = line;
	s->policy[s->used] = -1;
	++s->used;
}

int main(int argc, char **argv)
{
	//Read command line
	int vFlag = 0;
	int sValue = 0;
	int eValue = 0;
	int bValue = 0;
	char* tValue = NULL;
	int fFlag = 0;
	int lFlag = 0;
	int index;
	int c;

	while ((c = getopt (argc, argv, "hvs:v:E:b:t:FL")) != -1){
		switch (c){
			case 'h':
				printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile> (-L|-F)");
				return 1;
			case 'v':
				vFlag = 1;
				break;
			case 's':
				sValue = *optarg - '0';
				break;
			case 'E':
				eValue = *optarg - '0';
				break;
			case 'b':
				bValue = *optarg - '0';
				break;
			case 't':
				tValue = optarg;
				break;
			case 'F':
				fFlag = 1;
				break;
			case 'L':
				lFlag = 1;
				break;
			case '?':
				if (optopt == 's' || optopt == 'E' || optopt == 'b' || optopt == 't')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				return 1;
			default:
				abort ();
		}
	}
	if (fFlag != 1 && lFlag != 1){
		fprintf (stderr, "Specify either -F or -L\n");
		return 1;
	}

	// printf ("hFlag = %d, vFlag = %d, sValue = %d, eValue = %d, bValue = %d, tValue = %s, fFlag = %d, lFlag = %d\n", 
	// 	hFlag, vFlag, sValue, eValue, bValue, tValue, fFlag, lFlag);

	for (index = optind; index < argc; index++)
		printf ("Non-option argument %s\n", argv[index]);

	//Init cache
	int numSet = pow(2, sValue);
	int numLine = eValue;
	//int numBlock = pow(2, bValue);
	Cache cache; // = (Cache*) malloc(sizeof(Cache));
	initCache(&cache, 1);
	for (int i = 0; i < numSet; ++i){
		Set set;
		initSet(&set, 1);
		for (int j = 0; j < numLine; ++j){
			Line line;
			line.valid = 0;
			addLine(&set, line);
		}
		addSet(&cache, set);
	}

	//Open and read file
	FILE* fp;
	char line [50];
	fp = fopen(tValue, "r");
	if (fp == NULL){
		printf ("File does not exist\n");
		return 1;
	}

	unsigned long int tagMask = (unsigned long int) ((1LL << 63) >> (64 - bValue - sValue - 1));
	unsigned long int setMask = (unsigned long int) ((1LL << 63) >> (sValue - 1)) >> (64 - bValue - sValue);
	int hitCount = 0;
	int missCount = 0;
	int evictCount = 0;
	while (fgets (line, 50, fp) != NULL) {
		char fullLine[50];
		strcpy(fullLine, line);
		//Get command
		char* command = strtok(line, " ,");
		if (command[0] == 'I') continue;

		//Get command address
		char* addrStr = strtok(NULL, " ,");
		char *ptr;
		unsigned long int addr = strtol(addrStr, &ptr, 16);
		unsigned long int tag = (unsigned long int) (addr & tagMask) >> (bValue + sValue);
		unsigned long int setIndex = (unsigned long int) (addr & setMask) >> bValue;
		int hitFlag = 0;
		int evictFlag = 0;

		Set set = cache.sets[setIndex];
		for (int i = 0; i < set.used; ++i){
			if (set.lines[i].valid == 1){
				if (set.lines[i].tag == tag){
					hitFlag = 1;
					++hitCount;
					for (int j = 0; j < set.used; ++j){
						if (set.policy[j] >= 0) ++set.policy[j];
					}
					if (lFlag == 1){
						set.policy[i] = 0;
					}
					break;
				}
			}
		}
		if (hitFlag == 0) {
			++missCount;
			evictFlag = 1;
			int freeIndex = 0;
			for (int i = 0; i < set.used; ++i){
				if (set.lines[i].valid == 0){
					freeIndex = i;
					evictFlag = 0;
					break;
				}
			}
			if (evictFlag == 0) {
				for (int j = 0; j < set.used; ++j){
					if (set.policy[j] >= 0) ++set.policy[j];
				}
				set.lines[freeIndex].valid = 1;
				set.lines[freeIndex].tag = tag;
				set.policy[freeIndex] = 0; 
			}
			else {
				++evictCount;
				int evictIndex = 0;
				int max = 0;
				for (int j = 0; j < set.used; ++j){
					if (set.policy[j] > max){
						max = set.policy[j];
						evictIndex = j;
					}
				}
				for (int j = 0; j < set.used; ++j){
					if (set.policy[j] >= 0) ++set.policy[j];
				}
				set.lines[evictIndex].tag = tag;
				set.policy[evictIndex] = 0;
			}
		}
		if (command[0] == 'M'){
			hitCount++;
		}
		if (vFlag == 1){
			char text[] = "";
			char hitText[] = "hit ";
			char missText[] = "miss ";
			char evictText[] = "evict ";
			int len = strlen(fullLine);
			fullLine[len-1] = '\0';

			if (hitFlag == 1){
				//printf("hit!\n");
				strcat(text, hitText);
			}
			else {
				//printf("miss!\n");
				strcat(text, missText);
			}
			if (evictFlag == 1){
				//printf("evict!\n");
				strcat(text, evictText);
			}
			if (command[0] == 'M'){
				strcat(text, hitText);
			}
			printf ("%s %s\n", fullLine, text);
		}
	}	

	fclose(fp);
	for (int i = 0; i < cache.used; ++i){
		free(cache.sets[i].lines);
		free(cache.sets[i].policy);
	}
	free(cache.sets);
    printSummary(hitCount, missCount, evictCount);
    return 0;
}