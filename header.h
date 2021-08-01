//! This is some utilites for CS23 search engine project ( or general case)
//! \author wei pan (wei.pan@dartmouth.edu)
//! \brief useful MACROs for general C programming
// Filename: header.h
// Description: These are some useful MACRO for you. 

#ifndef _HEADER_H_
#define _HEADER_H_

#define min(x,y)   ((x)>(y))?(y):(x)

//! \brief Print \a s together with the source file name and the current line number.
#define LOG(s)  printf("[%s:%d]%s\n", __FILE__, __LINE__, s)


//! \brief Check whether \a s is NULL or not. Quit this program if it is NULL.
#define MYASSERT(s)  if (!(s))   {                                      \
    printf("General Assert Error at %s:line%d\n", __FILE__, __LINE__);  \
    exit(-1); \
  }


//! \brief Check whether \a s is NULL or not on a memory allocation. Quit this program if it is NULL.
#define MALLOC_CHECK(s)  if ((s) == NULL)   {                     \
    printf("No enough memory at %s:line%d ", __FILE__, __LINE__); \
    perror(":");                                                  \
    exit(-1); \
  }

//! \brief Set memory space starts at pointer \a n of size \a m to zero. 
#define BZERO(n,m)  memset(n, 0, m)



#endif