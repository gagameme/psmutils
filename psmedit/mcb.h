#ifndef MCB_H
#define MCB_H


#include "mc.h"


/* MCB header size */
#define SIZE_MCB_HEAD 54

/* max MCB file size (header size + data block size(max 15blocks)) */
#define SIZE_MCB_MAX (SIZE_MCB_HEAD + SIZE_BLOCK * NUM_DATABLOCK)


int mcb_read_file(const char *mcbfname, void *mem);

void *mcb_get_data(const void *mcb);

#endif


