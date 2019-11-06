/*
 * IRQ RTDM/UDD driver example for Raspberry Pi 3
 */
#include <linux/module.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <rtdm/driver.h>
#include <rtdm/udd.h>

// Pi 3
#define BCM2710_PERI_BASE   0x3F000000
#define GPIO_BASE           (BCM2710_PERI_BASE + 0x200000) /* GPIO controler */

#define RTDM_SUBCLASS_RPI_IRQ       0
#define DEVICE_NAME                 "rpi_irq"

// GPIO out/in
static int gpio_in = 17; // button/irq 

module_param(gpio_in, int, 0644);

MODULE_DESCRIPTION("UDD driver for RPI GPIO");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pierre Ficheux");

int irq_handler(struct udd_device *udd)
{
  rtdm_printk("RTDM driver: Got IRQ () !\n");

  return RTDM_IRQ_HANDLED;
}

static struct udd_device device = {
  .device_name = "rpi_irq",
  .device_flags		=	RTDM_NAMED_DEVICE|RTDM_EXCLUSIVE,
  .device_subclass = RTDM_SUBCLASS_RPI_IRQ,
  .ops = {
    .interrupt = irq_handler,
  },
};

int __init rpi_irq_init(void)
{
  int err;
  rtdm_printk("RPI_IRQ RTDM, loading\n");

  if ((err = gpio_request(gpio_in, THIS_MODULE->name)) != 0) {
    return err;
  }

  if ((err = gpio_direction_input(gpio_in)) != 0) {
    gpio_free(gpio_in);
    return err;
  }

  device.irq = gpio_to_irq(gpio_in);

  rtdm_printk("IRQ Number : %d\n",device.irq);

  irq_set_irq_type(device.irq, IRQF_TRIGGER_FALLING);

  return udd_register_device (&device); 
}

void __exit rpi_irq_exit(void)
{
  rtdm_printk("RPI_IRQ RTDM, unloading\n");

  udd_unregister_device (&device);
  gpio_free (gpio_in);
}

module_init(rpi_irq_init);
module_exit(rpi_irq_exit);
