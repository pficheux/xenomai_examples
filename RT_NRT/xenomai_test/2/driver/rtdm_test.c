/*
 * GPIO RTDM driver for _rt / _nrt test (Xenomai 2)
 */
#include <linux/module.h>
#include <linux/types.h>
#include <rtdm/rtdm_driver.h>

#define RTDM_SUBCLASS_RT_NRT_TEST       0
#define DEVICE_NAME                 "rtdm_test"

MODULE_DESCRIPTION("RTDM driver _rt / _nrt test");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pierre Ficheux");

static int rtdm_test_open(struct rtdm_dev_context *context, rtdm_user_info_t *user_info, int oflags)
{
  rtdm_printk("RTDM open\n");

  return 0;
}

static int rtdm_test_close(struct rtdm_dev_context *context, rtdm_user_info_t *user_info)
{
  rtdm_printk("RTDM close\n");

  return 0;
}

// called in non-RT context
static ssize_t rtdm_test_ioctl_nrt(struct rtdm_dev_context* context, rtdm_user_info_t* user_info, unsigned int request, void __user* arg)
{
  rtdm_printk("ioctl: NRT\n");

  return 0;
}

// called in RT context
static ssize_t rtdm_test_ioctl_rt(struct rtdm_dev_context* context, rtdm_user_info_t* user_info, unsigned int request, void __user* arg)
{
  rtdm_printk("ioctl: RT\n");

  return 0;
}

static struct rtdm_device device = {
 struct_version:         RTDM_DEVICE_STRUCT_VER,
 
 device_flags:           RTDM_NAMED_DEVICE,
 device_name:            DEVICE_NAME,

 open_nrt:               rtdm_test_open,

 ops:{
  close_nrt:      rtdm_test_close,
  
  ioctl_rt:       rtdm_test_ioctl_rt,
  ioctl_nrt:      rtdm_test_ioctl_nrt,
  },

 device_class:           RTDM_CLASS_SERIAL,
 device_sub_class:       RTDM_SUBCLASS_RT_NRT_TEST ,
 driver_name:            "rtdm_test",
 driver_version:         RTDM_DRIVER_VER(0, 0, 0),
 peripheral_name:        "RTDM_TEST",
 provider_name:          "PF",
 proc_name:              device.device_name,
};

int __init rtdm_test_init(void)
{
  rtdm_printk("rtdm test, loading\n");

  return rtdm_dev_register (&device); 
}

void __exit rtdm_test_exit(void)
{
  rtdm_printk("rtdm test, unloading\n");
  rtdm_dev_unregister (&device, 1000);
}

module_init(rtdm_test_init);
module_exit(rtdm_test_exit);
