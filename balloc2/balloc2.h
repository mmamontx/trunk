#ifndef BALLOC2_H
#define BALLOC2_H

#include <string.h>
#include <list>

#define BLOCK_FREE 0
#define BLOCK_USED 1

struct blockline {
  void *          ptr_first;
  size_t          size;
  unsigned int    idx_first;
  unsigned int    n;
  pthread_mutex_t mtx;
};

/**
 * balloc2 class defines block memory allocator objects.
 *
 * The objects are thread-safe with blockline granularity - meaning that it is
 * allowed to use other blocklines of the allocator while one of them is being used.
 * Note that although tsread() and tswrite() methods can be used for the purpose
 * of thread-safety (working with untampered blocklines), users can still
 * work with this memory in a regular fashion (i.e. for single-tasking experience).
 */
class balloc2 {
public:
  /**
   * Class constructor.
   *
   * \param bsize_req size of blocks in bytes.
   * \param bnum_req number of blocks.
   */
  balloc2( size_t bsize_req, int bnum_req ) {
    if ( ( bnum_req <= 0 ) || ( bsize_req == 0 ) ) {
      fprintf( stderr, "balloc(): bad parameters: bsize_req == %ld bnum_req == %d\n", bsize_req, bnum_req );
      memsize = 0;
      return;
    }

    bsize   = bsize_req;
    bnum    = bnum_req;
    memsize = bnum * bsize;

    membase = calloc( bnum, bsize );
    if ( membase == NULL ) {
      fprintf( stderr, "balloc(): calloc() returned NULL\n" );
      return;
    }

    // 1 byte per block - can be reduced to 1 bit per block
    map = ( unsigned char * )calloc( memsize, 1 );
    if ( map == NULL ) {
      fprintf( stderr, "balloc(): calloc() returned NULL\n" );
      free( membase );
      return;
    }

#if VERBOSE == 1
    printf( "balloc(): Created allocator containing %d blocks of %ld bytes of total size %ld at address %p.\n", bnum, bsize,
            memsize, membase );
#endif

    pthread_mutex_init( &( bbl ), NULL );
  }

  /**
   * Class destructor: frees the dynamically allocated memory on destruction to avoid memory leakage.
   */
  ~balloc2() {
    free( membase );
    free( map );
  }

  /**
   * bseek(): looks for a line of consecutive free blocks in the memory pool.
   *
   * \param bnum_req number of blocks.
   * \return Index of the first free block of the blockline, or -1.
   *
   * 1. Note that this method uses a greedy nested for loop of O(nm) - a list of free blocks can speedup this operation up to O(1).
   * 2. Limitation: Multiple free blocks can't be allocated as a continuous space if they are segmented (aren't located one after another). A better implementation may represent isolated chunks of memory as a single buffer.
   */
  int bseek( int bnum_req ) {
    int i, cnt;

    for ( i = 0, cnt = 0; i < bnum; i++ )
      if ( map[i] == BLOCK_FREE ) {
        cnt++;

        if ( cnt == bnum_req )
          return ( i + 1 ) - cnt; // Index of the first free block
      }
      else {
        cnt = 0;
      }

    return -1;
  }

  /**
   * balloc(): allocates memory from the pool.
   *
   * \param size_req memory size.
   * \return Pointer to the allocated memory.
   *
   * The method calculates the number of blocks required based on the requested memory
   * size. Then, if there is a space available, reserves the corresponding
   * blocks. Note that the requested size may not be aligned according to
   * the size of blocks. In this case allocator ceils it on its own. The
   * information about used memory blocks (lines of blocks) is stored in a
   * dedicated structure.
   */
  void * balloc( size_t size_req ) {
    if ( size_req == 0 ) {
      fprintf( stderr, "balloc(): bad parameter: size_req == 0\n" );
      return NULL;
    }

    if ( size_req > ( bsize * bnum ) ) {
      fprintf( stderr, "balloc(): bad parameter: size_req %ld is greater than total memsize == %ld\n", size_req, memsize );
      return NULL;
    }

    int bnum_req;
    if ( size_req % bsize )
      bnum_req = size_req / bsize + 1;
    else
      bnum_req = size_req / bsize;

    pthread_mutex_lock( &bbl );

    int idx_first = bseek( bnum_req );
    if ( idx_first == -1 ) {
      fprintf( stderr, "bseek() returned -1: can't find the requested number of free blocks (in a row).\n" );
      pthread_mutex_unlock( &bbl );
      return NULL;
    }

    for ( int i = 0; i < bnum_req; i++ )
      map[idx_first + i] = BLOCK_USED;

    void * addr = ( unsigned char * )membase + idx_first * bsize;

#if VERBOSE == 1
    printf( "balloc(): Allocated %d memory block(s) of size %ld at address %p.\n", bnum_req, bsize, addr );
#endif

    struct blockline bl;

    bl.ptr_first = addr;
    bl.size      = bnum_req * bsize;
    bl.idx_first = idx_first;
    bl.n         = bnum_req;

    pthread_mutex_init( &( bl.mtx ), NULL );

    blocklines.push_back( bl );

    pthread_mutex_unlock( &bbl );

    return addr;
  }

  /**
   * bfree(): frees memory block(s) requested earlier.
   *
   * \param ptr pointer to the beginning of the memory blockine.
   * \return 0 on success, or -1 on failure.
   *
   * Note that misaligned pointers as well as pointers out of bounds would cause an error.
   */
  int bfree( void * ptr ) {
    if ( ptr == NULL ) {
      fprintf( stderr, "bfree(): bad parameter ptr == NULL\n" );
      return -1;
    }

    if ( ptr < membase ) {
      fprintf( stderr, "ptr %p < membase %p: the requested address is out of range.\n", ptr, membase );
      return -1;
    }

    pthread_mutex_lock( &bbl );

    std::list< struct blockline >::iterator bl = blocklines.begin();
    while ( bl != blocklines.end() )
      if ( ( bl->ptr_first ) == ptr ) {
        // Found the corresponding blockline - mark its blocks free
        for ( unsigned int i = bl->idx_first; i < bl->idx_first + bl->n; i++ )
          map[i] = BLOCK_FREE;

#if VERBOSE == 1
        printf( "bfree(): Freed %d block(s) at address %p.\n", bl->n, ptr );
#endif

        pthread_mutex_destroy( &( bl->mtx ) );

        blocklines.erase( bl );

        pthread_mutex_unlock( &bbl );

        return 0;
      }
      else {
        bl++;
      }

    pthread_mutex_unlock( &bbl );

    fprintf( stderr, "Can't find blocklines starting at address %p.\n", ptr );

    return -1;
  }

  /**
   * is_empty(): checks if the memory pool is empty.
   *
   * \return true if empty, false - otherwise.
   */
  bool is_empty() { return blocklines.empty(); }

  /**
   * bclear(): clears all memory blocks and initializes them with 0's.
   */
  void bclear() {
    pthread_mutex_lock( &bbl );

    if ( membase )
      memset( membase, 0, bsize * bnum );
    if ( map )
      memset( map, 0, bnum );

    blocklines.clear();

    pthread_mutex_unlock( &bbl );

#if VERBOSE == 1
    printf( "bclear(): Erased all %d block(s).\n", bnum );
#endif
  }

  /**
   * get_memsize(): returns the total amount of memory reserved by the allocator.
   */
  unsigned long get_memsize() { return memsize; }

  /**
   * get_membase(): returns the base address of the memory pool.
   */
  void * get_membase() { return membase; }

  /**
   * tsread(): reads memory contents ensuring that it is not being tampered with at the moment.
   *
   * \param ptr a pointer to the place within the allocated memory.
   * \param buf a pointer to the outside buffer where the bytes should be written to.
   * \param n number of bytes.
   */
  size_t tsread( void * ptr, void * buf, size_t n ) {
    for ( struct blockline bl : blocklines )
      // Is the read within the boundaries?
      if ( ( ptr >= ( bl.ptr_first ) ) && ( ( ( unsigned char * )ptr + n ) <= ( unsigned char * )bl.ptr_first + bl.size ) ) {
        pthread_mutex_lock( &( bl.mtx ) );
        memcpy( buf, ( unsigned char * )ptr, n );
        pthread_mutex_unlock( &( bl.mtx ) );

        return n;
      }

    fprintf( stderr, "Read from %p outside the boundaries.\n", ptr );

    return 0;
  }

  /**
   * tswrite(): writes memory contents ensuring that it is not being tampered with at the moment.
   *
   * \param ptr a pointer to the place within the allocated memory where the bytes should be written to.
   * \param buf a pointer to the outside buffer where the bytes should be written from.
   * \param n number of bytes.
   */
  size_t tswrite( void * ptr, void * buf, size_t n ) {
    for ( struct blockline bl : blocklines )
      // Is the write within the boundaries?
      if ( ( ptr >= ( bl.ptr_first ) ) && ( ( ( unsigned char * )ptr + n ) <= ( unsigned char * )bl.ptr_first + bl.size ) ) {
        pthread_mutex_lock( &( bl.mtx ) );
        memcpy( ptr, buf, n );
        pthread_mutex_unlock( &( bl.mtx ) );

        return n;
      }

    fprintf( stderr, "Write to %p outside the boundaries.\n", ptr );

    return 0;
  }

private:
  size_t                        bsize;
  int                           bnum;
  unsigned long                 memsize;
  void *                        membase;
  unsigned char *               map;
  std::list< struct blockline > blocklines;
  pthread_mutex_t               bbl; // Big balloc lock: locks the structures containing metadata
};

#endif
