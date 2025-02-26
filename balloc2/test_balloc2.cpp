#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/random.h>
#include <sys/wait.h>

#include <gtest/gtest.h>

#include "balloc2.h"

#define BLOCK_SIZE 1024
#define BLOCK_NUM  16

#define N_THREADS 100

balloc2 * ba_shared;

TEST( balloc2Test, OutOfMemory ) {
  void *    ptr;
  balloc2 * ba;

  ba = new balloc2( BLOCK_SIZE, BLOCK_NUM );

  // Allocate all blocks
  ptr = ba->balloc( BLOCK_SIZE * BLOCK_NUM );

  // Try to allocate one more and ensure that it can't be done
  ptr = ba->balloc( BLOCK_SIZE * 1 );

  bool result = ( ptr == NULL ) ? 1 : 0;

  EXPECT_EQ( result, 1 );
}

TEST( balloc2Test, AllocateMisaligned ) {
  void *    ptr;
  balloc2 * ba;

  ba = new balloc2( BLOCK_SIZE, BLOCK_NUM );

  // Here comes an extra byte causing one more block to be allocated
  ba->balloc( BLOCK_SIZE * ( BLOCK_NUM - 1 ) + 1 );

  // Try to allocate one more and ensure that it can't be done
  ptr = ba->balloc( BLOCK_SIZE * 1 );

  bool result = ( ptr == NULL ) ? 1 : 0;

  EXPECT_EQ( result, 1 );
}

TEST( balloc2Test, Free ) {
  void *    ptr;
  balloc2 * ba;

  ba = new balloc2( BLOCK_SIZE, BLOCK_NUM );

  // Allocate all blocks
  ptr = ba->balloc( BLOCK_SIZE * BLOCK_NUM );

  ba->bfree( ptr );

  // Repeat
  ptr = ba->balloc( BLOCK_SIZE * BLOCK_NUM );

  bool result = ( ptr != NULL ) ? 1 : 0;

  EXPECT_EQ( result, 1 );
}

TEST( balloc2Test, FreeMisaligned ) {
  void *    ptr;
  balloc2 * ba;
  int       result = -1;

  ba = new balloc2( BLOCK_SIZE, BLOCK_NUM );

  // Allocate all blocks
  ptr = ba->balloc( BLOCK_SIZE * BLOCK_NUM );

  if ( BLOCK_SIZE > 1 )
    result = ba->bfree( ( unsigned char * )ptr + 1 );

  EXPECT_EQ( result, -1 );
}

void * user_thread( void * ptr ) {
  ba_shared->balloc( BLOCK_SIZE ); // Request a single block

  return NULL;
}

TEST( balloc2Test, ConcurrentBlockAllocation ) {
  void * ptr;
  bool   result;

  ba_shared = new balloc2( BLOCK_SIZE, N_THREADS ); // 1 block per thread

  pthread_t user_threads[N_THREADS];

  for ( int i = 0; i < N_THREADS; i++ )
    pthread_create( &( user_threads[i] ), NULL, user_thread, NULL );

  for ( int i = 0; i < N_THREADS; i++ )
    pthread_join( user_threads[i], NULL );

  // Request one more block: if it's there it means that the allocation logic
  // got overlapped, at some point leading to some of the threads getting an
  // identical block (in a best scenario).
  ptr = ba_shared->balloc( BLOCK_SIZE ); // Request a single block

  delete ba_shared;

  result = ( ptr == NULL ) ? 1 : 0;

  EXPECT_EQ( result, 1 );
}

void * user_thread2( void * ptr ) {
  unsigned int thread_id = *( ( unsigned int * )ptr );
  void *       membase   = ba_shared->get_membase();

  ba_shared->bfree( ( unsigned char * )membase + BLOCK_SIZE * thread_id );

  return NULL;
}

TEST( balloc2Test, ConcurrentBlockDeallocation ) {
  bool         result;
  unsigned int thread_id[N_THREADS];

  ba_shared = new balloc2( BLOCK_SIZE, N_THREADS ); // 1 block per thread

  for ( int i = 0; i < N_THREADS; i++ )
    ba_shared->balloc( BLOCK_SIZE );

  pthread_t user_threads[N_THREADS];

  for ( unsigned int i = 0; i < N_THREADS; i++ ) {
    thread_id[i] = i;
    pthread_create( &( user_threads[i] ), NULL, user_thread2, thread_id + i );
  }

  for ( int i = 0; i < N_THREADS; i++ )
    pthread_join( user_threads[i], NULL );

  // By this moment all the blocks must be freed
  result = ba_shared->is_empty();

  EXPECT_EQ( result, 1 );
}
