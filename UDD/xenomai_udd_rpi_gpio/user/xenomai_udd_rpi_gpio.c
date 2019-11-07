/* 
 * Xenomai example for GPIO on Pi (POSIX skin + UDD)
 */

#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/io.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <ctype.h>
#include <rtdm/udd.h>
#include <cobalt/sys/cobalt.h>

#define NSEC_PER_SEC    1000000000

/* the struct timespec consists of nanoseconds
 * and seconds. if the nanoseconds are getting
 * bigger than 1000000000 (= 1 second) the
 * variable containing seconds has to be
 * incremented and the nanoseconds decremented
 * by 1000000000.
 */
static inline void tsnorm(struct timespec *ts)
{
   while (ts->tv_nsec >= NSEC_PER_SEC) {
      ts->tv_nsec -= NSEC_PER_SEC;
      ts->tv_sec++;
   }
}

// I/O access
volatile unsigned *gpio;

#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
// For GPIO# >= 32 (RPi B+)
#define GPIO_SET_EXT *(gpio+8)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR_EXT *(gpio+11) // clears bits which are 1 ignores bits which are 0

void gpio_set (int g)
{
  if (g >= 32)
    GPIO_SET_EXT = (1 << (g % 32));
  else
    GPIO_SET = (1 << g);
}

void gpio_clr (int g)
{
  if (g >= 32)
    GPIO_CLR_EXT = (1 << (g % 32));
  else
    GPIO_CLR = (1 << g);
}

int  fd, fd_mapper0;
char *gpio_map;
pthread_t thid_square, thid_irq;

#define PERIOD          100000000 // default is 100 ms
int nibl = 0;
int gpio_nr = 4; // default is led

unsigned long period_ns = 0;
int loop_prt = 100;             /* print every 100 loops: 5 s */ 
unsigned int test_loops = 0;    /* outer loop count */

/* Thread function (IRQ) */
void *thread_irq (void *dummy)
{
  struct udd_signotify udd_signotify;
  sigset_t set;
  int ret, sig, pid;
  struct timespec timeout;
  siginfo_t siginfo; 

  pid = cobalt_thread_pid(pthread_self());
  
  printf ("IRQ thread %d starting\n", pid);

  udd_signotify.pid = pid;
  udd_signotify.sig = SIGRTMIN + 1;

  timeout.tv_sec = 2;
  timeout.tv_nsec = 0;
  sigemptyset(&set);
  sigaddset(&set, SIGRTMIN + 1);

  // Configure IRQ
  if ((fd = open("/dev/rtdm/rpi_gpio", O_RDWR)) < 0) {
    perror ("open rpi_gpio");
    exit (1);
  }
  printf ("%s: fd= %d\n", __FUNCTION__, fd);
  
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


/* Thread function*/
void *thread_square (void *dummy)
{
  struct timespec ts, tr;
  time_t t = 0, told = 0, jitter;
  static time_t jitter_max = 0, jitter_avg = 0;

  // Get UDD addr for GPIO controler

  // Open memory with mmap()
  if ((fd_mapper0 = open("/dev/rtdm/rpi_gpio,mapper0", O_RDWR)) < 0) {
      perror("open mapper0");
      exit(1);
  }
  
  gpio_map = (char *)mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mapper0, 0);
  close(fd_mapper0);

  if (gpio_map == MAP_FAILED) {
    perror ("mmap");
    exit(1);
  }

  // Always use volatile pointer!
  gpio = (volatile unsigned *)gpio_map;

  // Set GPIO  as output
  OUT_GPIO(gpio_nr);
  
  // Get current time
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec++;
  
  /* Main loop */
  for (;;)
    {
      // Absolute wait
      if (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL)) {
	perror ("clock_nanosleep");
	exit (1);
      }

      /* Write to GPIO */
      if (test_loops % 2)
	gpio_set (gpio_nr);
      else
	gpio_clr (gpio_nr);

      // wait 'period' ns
      ts.tv_nsec += period_ns;
      tsnorm (&ts);
      
      /* old_time <-- current_time */
      told = t;

      /* get current time */
      clock_gettime (CLOCK_REALTIME, &tr);
      t = (tr.tv_sec * 1000000000) + tr.tv_nsec;    

      // Calculate jitter + display
      jitter = abs(t - told - period_ns);
      jitter_avg += jitter;
      if (test_loops && (jitter > jitter_max))
	jitter_max = jitter;
     
      if (test_loops && !(test_loops % loop_prt)) {
	jitter_avg /= loop_prt;
	printf ("Loop= %d sec= %ld nsec= %ld delta= %ld ns jitter cur= %ld ns avg= %ld ns max= %ld ns\n", test_loops,  tr.tv_sec, tr.tv_nsec, t-told, jitter, jitter_avg, jitter_max);
	jitter_avg = 0;
      }

      test_loops++;
    }
}

void cleanup_upon_sig(int sig __attribute__((unused)))
{
  pthread_cancel (thid_square);
  pthread_join (thid_square, NULL);
  exit(0);
}

void usage (char *s)
{
  fprintf (stderr, "Usage: %s [-p period (ns)] [-g gpio#]\n", s);
  exit (1);
}


int main (int ac, char **av)
{
  int err, c;
  char *progname = (char*)basename(av[0]);
  struct sched_param param_square = { .sched_priority = 99 };
  pthread_attr_t thattr;

  period_ns = PERIOD; /* ns */

  signal(SIGINT, cleanup_upon_sig);
  signal(SIGTERM, cleanup_upon_sig);
  signal(SIGHUP, cleanup_upon_sig);
  signal(SIGALRM, cleanup_upon_sig);

  while ((c = getopt (ac, av, "hp:g:")) != -1) {
    switch (c)
      {
      case 'g':
	gpio_nr = atoi(optarg);
	break;
	
      case 'p':
	period_ns = atoi(optarg);
	break;

      case 'h':
	usage(progname);
	
      default:
	break;
      }
  }

  // Display every 2 sec
  loop_prt = 2000000000 / period_ns;
  
  printf ("Using GPIO %d and period %ld ns\n", gpio_nr, period_ns);

  // Thread attributes
  pthread_attr_init(&thattr);

  // Priority, set priority to 99
  pthread_attr_setinheritsched(&thattr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&thattr, SCHED_FIFO);
  pthread_attr_setschedparam(&thattr, &param_square);

  // Create thread
  err = pthread_create(&thid_square, &thattr, &thread_square, NULL);

  if (err)
    {
      fprintf(stderr,"square: failed to create square thread, code %d\n",err);
      return 0;
    }

  // Create thread (IRQ)
  param_square.sched_priority--;
  pthread_attr_setschedparam(&thattr, &param_square);
  err = pthread_create(&thid_irq, &thattr, &thread_irq, NULL);

  if (err)
    {
      fprintf(stderr,"irq: failed to create IRQ thread, code %d\n",err);
      return 0;
    }

  pause();

  return 0;
}

