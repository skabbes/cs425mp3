#ifndef MUTUAL_EXCLUSION_H
#define MUTUAL_EXCLUSION_H

#include <pthread.h>
#include <vector>
using namespace std;

pthread_t startDetachedThread( void * (*functor)(void *), void * arg );
pthread_t startThread( void * (*functor)(void *), void * arg );
void mutual_exclusion_init( vector<int> const & nodes, vector<int> const & ports, int id );
void mutual_exclusion_destroy();
void mutual_exclusion_init();
void * tokenPasser( void * arg );
void lock();
void unlock();

#endif
