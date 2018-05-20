#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <flashLib.h>

#define TRACE(var) fprintf(stderr, #var" = %d 0x%x\n", var, var)

#define FCOUNT    8
#define BLOCKSIZE 524288

struct addr_list {
	int *addr;
	struct addr_list *next;
};

struct binfile {
	char *name;
	struct addr_list *addr;
};

int assign(struct binfile *list, int *addr, short count)
{
	struct addr_list *ptr;
	short i;
	
	list->addr = (struct addr_list *)malloc(sizeof(struct addr_list));
	
	if (list->addr == NULL) {
		fprintf(stderr, "malloc() returned NULL\n");
		
		return -1;
	}
	
	list->addr->addr = (int *)addr[0];
	
	for (i = 1, ptr = list->addr; i < count; i++) {
		ptr->next = (struct addr_list *)malloc(sizeof(struct addr_list));
		
		if (ptr->next == NULL) {
			fprintf(stderr, "malloc() returned NULL\n");
			
			return -1;
		} else {
			ptr = ptr->next;
			ptr->addr = (int *)addr[i];
		}
	}
	
	ptr->next = NULL;
	
	return 0;
}

struct binfile *fill()
{
	int addr[FCOUNT][4] = {
	{0xba000000, 0xba800000, 0xbb800000, 0xbc000000},
	{0xba100000, 0xba900000, 0xbb900000, 0xbc100000},
	{0xba180000, 0xba980000, 0xbb980000, 0xbc180000},
	{0xba200000, 0xbaa00000, 0xbba00000, 0xbc200000},
	{0xba300000, 0xbaa80000, 0xbb880000, 0xbc080000},
	{0xba380000, 0xbab00000, 0xbba80000, 0xbc280000},
	{0xba400000, 0xbab80000, 0xbbb00000, 0xbc300000},
	{0xba480000, 0xbac00000, 0xbbb80000, 0xbc380000}};
	char *name[FCOUNT] = {"/nfs/pz", "/nfs/vci.bin", "/nfs/pzk.bin", "/nfs/rzk.bin", "/nfs/pzoesk.bin", "/nfs/kv", "/nfs/nv", "/nfs/nu"};
	short i;
	struct binfile *list;
	
	list = (struct binfile *)malloc(sizeof(struct binfile) * FCOUNT);
	
	if (list == NULL) {
		fprintf(stderr, "malloc() returned NULL\n");
		
		return NULL;
	}
	
	for (i = 0; i < FCOUNT; i++) {
		list[i].name = name[i];
		
		if (assign(&(list[i]), addr[i], 4) == -1) {
			fprintf(stderr, "assign() returned -1\n");
			
			return NULL;
		}
	}
	
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

int isempty(unsigned short *buf, unsigned int len)
{
	unsigned int i;
	
	for (i = 0; i < len; i++)
		if (buf[i] != 0xffff)
			return FALSE;
	
	return TRUE;
}

int main()
{
	int rv;
	char tail;
	char itoa[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	char filename[16] = {0};
	short step, j;
	unsigned int len;
	unsigned short *buf;
	struct binfile *list;
	struct addr_list *ptr;
	FILE *output;
	
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
	
	if ((buf = (unsigned short *)calloc(BLOCKSIZE, 1)) == NULL) {
		fprintf(stderr, "calloc() returned NULL\n");
		
		release(list);
		
		return -1;
	}
	
	for (step = 0; step < FCOUNT; step++)
		for (ptr = list[step].addr, j = 1; ptr != NULL; ptr = ptr->next, j++, len = 0) {
			if (step < FCOUNT - 3) {
				if ((rv = flashReadBlock((char *)buf, (char *)ptr->addr, 6)) != 0) {
					fprintf(stderr, "flashReadBlock() returned %d\n", rv);
					
					free(buf);
					
					release(list);
					
					return -1;
				}
				
				if (buf[0] != 0xffff) {
					len = (buf[1] & 0x00ff) << 8;
					len += (buf[1] & 0xff00) >> 8;
					
					len <<= 16;
					
					len += (buf[0] & 0x00ff) << 8;
					len += (buf[0] & 0xff00) >> 8;
					
					TRACE(len);
					
					if (len > BLOCKSIZE) {
						fprintf(stderr, "len > %d\n", BLOCKSIZE);
						
						continue;
					}
					
					if ((rv = flashReadBlock((char *)buf, (char *)ptr->addr, len)) != 0) {
						fprintf(stderr, "flashReadBlock() returned %d\n", rv);
						
						free(buf);
						
						release(list);
						
						return -1;
					}
				}
			} else {
				switch (step) {
				case FCOUNT - 3:
					len = 19;
					break;
				case FCOUNT - 2:
					len = 13;
					break;
				case FCOUNT - 1:
					len = 48;
					break;
				default:
					TRACE(step);
					return -1;
				}
				
				if ((rv = flashReadBlock((char *)buf, (char *)ptr->addr, sizeof(int) * len)) != 0) {
					fprintf(stderr, "flashReadBlock() returned %d\n", rv);
					
					free(buf);
					
					release(list);
					
					return -1;
				}
				
				if (isempty(buf, len))
					continue;
			}
			
			if ((len > 0) || (step > FCOUNT - 3)) {
				memset(filename, 0, 16);
				
				strcat(filename, list[step].name);
				
				tail = itoa[j];
				
				strcat(filename, &tail);
				
				puts(filename);
				
				if ((output = fopen(filename, "wb")) == NULL) {
					fprintf(stderr, "fopen() returned NULL\n");
					
					free(buf);
					
					release(list);
					
					return -1;
				}
				
				fwrite(buf, 1, len, output);
				
				fclose(output);
				
				memset(buf, 0, len);
			}
		}
	
	free(buf);
	
	release(list);
	
	return 0;
}
