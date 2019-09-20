/*
 * GPIO RTDM driver for _rt / _nrt test (Xenomai 3)
 */
#include <linux/module.h>
#include <linux/types.h>
#include <rtdm/driver.h>

#define RTDM_SUBCLASS_RT_NRT_TEST   0
#define DEVICE_NAME                 "rtdm_test"

MODULE_DESCRIPTION("RTDM driver _rt / _nrt test");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pierre Ficheux");

static int debug;

module_param(debug, int, 0644);

static int rtdm_test_open(struct rtdm_fd *fd, int oflags)
{
  rtdm_printk("RTDM open:.\n");

  return 0;
}

static void rtdm_test_close(struct rtdm_fd *fd)
{
  rtdm_printk("RTDM close:.\n");

  return;
}

// called in non-RT context
static ssize_t rtdm_test_ioctl_nrt(struct rtdm_fd *fd, unsigned int request, void *arg)
{
  rtdm_printk("ioctl: NRT\n");

  return 0;
}

// called in RT context
static ssize_t rtdm_test_ioctl_rt(struct rtdm_fd *fd, unsigned int request, void *arg)
{
  rtdm_printk("ioctl: RT\n");

  return 0;
}

static struct rtdm_driver rtdm_test_driver = {
  .profile_info		=	RTDM_PROFILE_INFO(autotune,
						  RTDM_CLASS_SERIAL,
						  RTDM_SUBCLASS_RT_NRT_TEST,
						  0),
  .device_flags		=	RTDM_NAMED_DEVICE|RTDM_EXCLUSIVE,
  .device_count		=	1,
  .ops = {
    .open		=	rtdm_test_open,
    .ioctl_rt	        =	rtdm_test_ioctl_rt,
    .ioctl_nrt	        =	rtdm_test_ioctl_nrt,
    .close		=	rtdm_test_close,
  },
};

static struct rtdm_device device = {
  .driver = &rtdm_test_driver,
  .label = "rtdm_test",
};

int __init rtdm_test_init(void)
{
  rtdm_printk("rtdm test, loading\n");

  return rtdm_dev_register (&device); 
}

void __exit rtdm_test_exit(void)
{
  rtdm_printk("rtdm test, unloading\n");
  rtdm_dev_unregister (&device);
}

module_init(rtdm_test_init);
module_exit(rtdm_test_exit);
