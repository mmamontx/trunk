#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <flashLib.h>

#define TRACE(var) fprintf(stderr, #var" = %d 0x%x\n", var, var)

#define SHIFT  4
#define FCOUNT 15

struct addr_list {
	int *addr;
	struct addr_list *next;
};

struct binfile {
	char *name;
	unsigned short csum;
	struct addr_list *addr;
};

int last;
unsigned short *buf;

/*
 * Флаг   shift  устанавливается  если  требуется  эмулировать  смещение
 * аналогично lbf:  размер  округляется  до четного в большую сторону, к
 * нему  прибавляется  пара  байтов и он округляется до кратного четырем
 * в большую сторону.
 */
unsigned short csum16(int *start, int size, char shift)
{
	int i;
	int size_shift;/* Остаток деленного на shift размера. */
	unsigned int csum = 0;
	unsigned short *ptr = (unsigned short *)start;
	
	if ((buf = (unsigned short *)malloc(sizeof(short) * size / 2 + 7)) == NULL) {
		fprintf(stderr, "malloc() returned NULL\n");
		
		return -1;
	}
	
	if (shift) {
		if (size % 2)
			size++;
		
		size += 2;
		size_shift = abs(size % SHIFT - 4);
		size += size_shift;
	}
	
	if (start == NULL) {
		fprintf(stderr, "start == NULL\n");
		
		exit(-1);
	}
	
	if (size <= 0) {
		TRACE(size);
		
		exit(-1);
	}
	
	for (i = 0; i < size / 2; i++) {
		csum += ptr[i];
		buf[i] = ptr[i];
	}
	
	last = i - 1;
	
	csum = csum << 16;
	csum = csum >> 16;
	
	return (unsigned short)csum;   
}

struct binfile *fill()
{
	struct binfile *list;
	
	list = (struct binfile *)malloc(sizeof(struct binfile) * FCOUNT);
	
	if (list == NULL) {
		fprintf(stderr, "malloc() returned NULL\n");
		
		return NULL;
	}
	
	list[0].name = "bin/oc2000.bin";
	list[0].csum = 0x8a04;
	list[0].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[0].addr->addr = (int *)0xb8000000;
	list[0].addr->next = NULL;
	
	list[1].name = "bin/20029/oc2000.bin";
	list[1].csum = 0xc1d8;
	list[1].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[1].addr->addr = (int *)0xb8180000;
	list[1].addr->next = NULL;
	
	list[2].name = "bin/20361/oc2000.bin";
	list[2].csum = 0xc1ad;
	list[2].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[2].addr->addr = (int *)0xb8380000;
	list[2].addr->next = NULL;
	
	list[3].name = "bin/pz";
	list[3].csum = 0xf694;
	list[3].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[3].addr->addr = (int *)0xba000000;
	list[3].addr->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[3].addr->next->addr = (int *)0xba800000;
	list[3].addr->next->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[3].addr->next->next->addr = (int *)0xbb000000;
	list[3].addr->next->next->next = NULL;
	
	list[4].name = "bin/PSU";
	list[4].csum = 0xfd6d;
	list[4].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[4].addr->addr = (int *)0xba080000;
	list[4].addr->next = NULL;
	
	list[5].name = "bin/vci.bin";
	list[5].csum = 0xffec;
	list[5].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[5].addr->addr = (int *)0xba100000;
	list[5].addr->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[5].addr->next->addr = (int *)0xba900000;
	list[5].addr->next->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[5].addr->next->next->addr = (int *)0xbb100000;
	list[5].addr->next->next->next = NULL;
	
	list[6].name = "bin/pzk.bin";
	list[6].csum = 0xff9a;
	list[6].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[6].addr->addr = (int *)0xba180000;
	list[6].addr->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[6].addr->next->addr = (int *)0xba980000;
	list[6].addr->next->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[6].addr->next->next->addr = (int *)0xbb180000;
	list[6].addr->next->next->next = NULL;
	
	list[7].name = "bin/rzk.bin";
	list[7].csum = 0xe870;
	list[7].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[7].addr->addr = (int *)0xba200000;
	list[7].addr->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[7].addr->next->addr = (int *)0xbaa00000;
	list[7].addr->next->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[7].addr->next->next->addr = (int *)0xbb200000;
	list[7].addr->next->next->next = NULL;
	
	list[8].name = "bin/almanach.arc";
	list[8].csum = 0xfa31;
	list[8].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[8].addr->addr = (int *)0xba280000;
	list[8].addr->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[8].addr->next->addr = (int *)0xbb280000;
	list[8].addr->next->next = NULL;
	
	list[9].name = "bin/pzoesk.bin";
	list[9].csum = 0xd317;
	list[9].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[9].addr->addr = (int *)0xba300000;
	list[9].addr->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[9].addr->next->addr = (int *)0xbaa80000;
	list[9].addr->next->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[9].addr->next->next->addr = (int *)0xbb300000;
	list[9].addr->next->next->next = NULL;
	
	list[10].name = "bin/psu";
	list[10].csum = 0xfced;
	list[10].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[10].addr->addr = (int *)0xbb080000;
	list[10].addr->next = NULL;
	
	list[11].name = "bin/kpa_bins_b62.bin";
	list[11].csum = 0x2eaa;
	list[11].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[11].addr->addr = (int *)0xbc000000;
	list[11].addr->next = NULL;
	
	list[12].name = "bin/osrvlib_b62.img";
	list[12].csum = 0x00bc;
	list[12].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[12].addr->addr = (int *)0xbc800000;
	list[12].addr->next = NULL;
	
	list[13].name = "bin/osrvlib_b62_ut.img";
	list[13].csum = 0x087d;
	list[13].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[13].addr->addr = (int *)0xbcc00000;
	list[13].addr->next = NULL;
	
	list[14].name = "bin/blank";
	list[14].csum = 0xfff5;
	list[14].addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[14].addr->addr = (int *)0xbf000000;
	list[14].addr->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[14].addr->next->addr = (int *)0xbf800000;
	list[14].addr->next->next = (struct addr_list *)malloc(sizeof(struct addr_list));
	list[14].addr->next->next->addr = (int *)0xbf880000;
	list[14].addr->next->next->next = NULL;
	
	return list;
}

int release(struct binfile *list)
{
	short i;
	struct addr_list *ptr, *tmp;
	
	for (i = 0; i < FCOUNT; i++) {
		ptr = list[i].addr;
		
		while (ptr->next != NULL) {
			tmp = ptr->next;
			
			free(ptr);
			
			ptr = tmp;
		}
		
		free(ptr);
	}
	
	free(list);
	
	return 0;
}

int main()
{
	int sec, ssec, rv, block, offset, count, i;
	int *base;
	short step;
	unsigned short csum;
	struct stat info;
	struct binfile *list;
	struct addr_list *ptr;
	
	printf("\n");
	printf("\t////   //   //    //   //  //   //  //   //    ///    ///////   //\n");
	printf("\t///   //   //    //   //  //  ///  //  ///   // //   //        //\n");
	printf("\t//   //   //    //   //  // ////  // ////  //   //  //        // /\n");
	printf("\t/   //   //    ///////  //// //  //// //  ///////  //        // //\n");
	printf("\t   //   //    //   //  ///  //  ///  //  //   //  //        // ///\n");
	printf("\t  /////////  //   //  //   //  //   //  //   //  //        // ////\n");
	printf("\t        //                                                // /////\n");
	printf("\t/////////////////////////////////////////////////////////// //////\n");
	printf("\n");
	
	if ((list = fill()) == NULL) {
		fprintf(stderr, "fill() returned NULL\n");
		
		return -1;
	}
	
	flashInfo(&sec, &ssec, &base);
	
	for (step = 0; step < FCOUNT; step++) {
		printf("%s\n", list[step].name);
		
		for (ptr = list[step].addr; ptr != NULL; ptr = ptr->next) {
			stat(list[step].name, &info);
			
			offset = (int)ptr->addr - (int)base;
			block = offset / ssec;
			count = info.st_size / ssec + 1;
			
			for (i = 0; i < count; i++, block++)
				if ((rv = flashEraseSector(block)) != 0) {
					fprintf(stderr, "flashEraseSector() returned %d\n", rv);
					
					release(list);
					
					return -1;
				}
			
			if ((rv = flashWriteFile(list[step].name, (char *)ptr->addr, FALSE, TRUE)) != 0) {
				fprintf(stderr, "flashWriteFile() returned %d\n", rv);
				
				release(list);
				
				return -1;
			}
			
			if ((csum = csum16(ptr->addr, (int)info.st_size, TRUE)) != list[step].csum) {
				printf("%04x != %04x\n", csum, list[step].csum);
				
				buf[last] += list[step].csum - csum;
				
				block = offset / ssec;
				
				for (i = 0; i < count; i++, block++)
					if ((rv = flashEraseSector(block)) != 0) {
						fprintf(stderr, "flashEraseSector() returned %d\n", rv);
						
						free(buf);
						
						release(list);
						
						return -1;
					}
				
				if ((rv = flashWriteBlock((char *)ptr->addr, (char *)buf, (int)info.st_size + 7, FALSE, FALSE)) != 0) {
					fprintf(stderr, "flashWriteBlock() returned %d\n", rv);
					
					free(buf);
					
					release(list);
					
					return -1;
				}
				
				free(buf);
				
				if ((csum = csum16(ptr->addr, (int)info.st_size, TRUE)) != list[step].csum) {
					fprintf(stderr, "%04x != %04x\n", csum, list[step].csum);
					
					free(buf);
					
					release(list);
					
					return -1;
				} else printf("%04x == %04x\n", csum, list[step].csum);
				
				free(buf);
			}
			
			puts("~~~~~~~~~~~~~~~~~~~~~~");
		};
	}
	
	release(list);
	
	return 0;
}
