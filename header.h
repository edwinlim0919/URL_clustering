#ifndef _HEADER_H_
#define _HEADER_H_


#define min(x,y)   ((x)>(y))?(y):(x)
#define LOG(s)  printf("[%s:%d]%s\n", __FILE__, __LINE__, s)


#define MYASSERT(s)  if (!(s))   {                                      \
    printf("General Assert Error at %s:line%d\n", __FILE__, __LINE__);  \
    exit(-1); \
  }


#define MALLOC_CHECK(s)  if ((s) == NULL)   {                     \
    printf("No enough memory at %s:line%d ", __FILE__, __LINE__); \
    perror(":");                                                  \
    exit(-1); \
  }


#define BZERO(n,m)  memset(n, 0, m)


#endif