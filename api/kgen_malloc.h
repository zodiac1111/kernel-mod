
#ifndef __KGEN_MALLOC_H__
#define __KGEN_MALLOC_H__

/* kmalloc support max 128KByte */
#define KGEN_KMALLOC_MAX_SIZE (128*1024)

char *kgen_malloc(unsigned int iLen);
char *kgen_malloc_dma(unsigned int iLen);
void kgen_free(void *ptr);
char *kgen_malloc_pages(unsigned int iLen);
char *kgen_malloc_dma_pages(unsigned int iLen);
void kgen_free_pages(void *ptr, unsigned int iLen);

#endif


