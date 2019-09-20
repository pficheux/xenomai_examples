/* 
 * Xenomai 3 test for double scheduling (_rt / _nrt)
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>

int fd;
pthread_t thid_rt;

void *thread_rt (void *dummy)
{
  struct timespec ts;

  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec++;

  while (1) {
    if (ioctl(fd, 0, 0) < 0) {
      perror ("rt_dev_ioctl");
      exit (1);
    }
  
    if (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL)) {
      perror ("clock_nanosleep");
      exit (1);
    }
    
    ts.tv_sec++;
  }	
}

int main (int ac, char **av)
{
  int err;
  pthread_attr_t thattr_rt;
  struct sched_param param_rt = {.sched_priority = 99 };
  
  // Open RTDM driver
  if ((fd = open("/dev/rtdm/rtdm_test", 0)) < 0) 
    {
      perror("rt_open");
      exit(EXIT_FAILURE);
    }
  // Thread attributes
  pthread_attr_init(&thattr_rt);
  
  // Joinable 
  pthread_attr_setdetachstate(&thattr_rt, PTHREAD_CREATE_JOINABLE);

  // Priority, set priority to 99
  pthread_attr_setinheritsched(&thattr_rt, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&thattr_rt, SCHED_FIFO);
  pthread_attr_setschedparam(&thattr_rt, &param_rt);

  // Create thread 
  err = pthread_create(&thid_rt, &thattr_rt, &thread_rt, NULL);
  if (err)
    {
      fprintf(stderr,"failed to create rt thread, code %d\n",err);
      return 0;
    }

  while (1) {
    if (ioctl(fd, 0, 0) < 0) {
      perror ("rt_dev_ioctl");
      exit (1);
    }

    sleep(1);	
  }
  
  return 0;
}
