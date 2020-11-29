
#ifndef __BOOGA_H
#define __BOOGA_H

#ifndef BOOGA_MAJOR
#define BOOGA_MAJOR 0   /* dynamic major */
#endif

#ifndef BOOGA_NR_DEVS
#define BOOGA_NR_DEVS 4 
#endif

#define TYPE(dev)   (MINOR(dev) >> 4)  /* high nibble */
#define NUM(dev)    (MINOR(dev) & 0xf) /* low  nibble */

struct booga_stats {
	long int num_open;
	long int num_read; 
	long int num_write; 
	long int num_close; 
	long int device_num;	
	long int numOfStrings[4];
	long int numOfOpened[4];
	struct semaphore sem;
};
typedef struct booga_stats booga_stats;

#endif