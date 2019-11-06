/* 
 * Simple Alchemy (native) task
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <execinfo.h>
#include <alchemy/task.h>
#include <rtdm/udd.h>

void task_body (void *cookie)
{
  struct udd_signotify udd_signotify;
  sigset_t set;
  int ret, sig, fd;
  struct timespec timeout;
  siginfo_t siginfo; 
  RT_TASK_INFO info;
	
  printf ("IRQ task starting\n");

  rt_task_inquire(NULL, &info);
  udd_signotify.pid = info.pid;
  udd_signotify.sig = SIGRTMIN + 1;

  timeout.tv_sec = 2;
  timeout.tv_nsec = 0;
  sigemptyset(&set);
  sigaddset(&set, SIGRTMIN + 1);

  // Configure IRQ
  if ((fd = open("/dev/rtdm/rpi_irq", O_RDWR)) < 0) {
    perror ("open");
    exit (1);
  }

  ret = ioctl(fd, UDD_RTIOC_IRQSIG, &udd_signotify);
  if (ret < 0)
    fprintf(stderr, "failed to enable signotify (%d)\n", ret);
  
  ret = ioctl(fd, UDD_RTIOC_IRQEN);
  if (ret < 0)
    fprintf(stderr, "failed to enable interrupt (%d)\n", ret);

  /* Wait for IRQ (TBC) */
  while (1) {
    // wait next IRQ
    sig = sigtimedwait(&set, &siginfo, &timeout);
    if (sig > 0)
      printf ("%s: Got IRQ (signal = %d)\n", __FUNCTION__, sig);
  }
}

int main (int ac, char **av)
{
  int err;
  RT_TASK task_desc;
  
  err = rt_task_spawn(&task_desc, "my_task", 0, 99, T_JOINABLE, &task_body, NULL);
  if (err) {
    fprintf(stderr,"failed to spawn task, code %d\n",err);
    return 0;
  }

  rt_task_join(&task_desc);

  return 0;
}
