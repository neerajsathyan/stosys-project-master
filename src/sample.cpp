/* Thread Creation */
#include <pthread.h>
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void*), void *arg);

/*Wait for completion*/
int pthread_join(pthread_t thread, void **value_ptr);

/*Locks*/
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

/*shared variable by all threads*/
volatile int varname;
