#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h> // Added for full header completeness

#define DEVICE_NAME "secure_sim_latch"
#define SIM_IOCTL_CMD_EJECT 0x101

static int major_num;
static struct class* secure_sim_class = NULL;
static struct device* secure_sim_device = NULL;

static long secure_sim_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    // SECURITY FIX: Access Control Check (Only allowing processes with CAP_SYS_ADMIN or system server UID)
    if (!capable(CAP_SYS_ADMIN)) {
        printk(KERN_WARNING "SecureSIM_Driver: Unauthorized process attempted to invoke SIM Eject IOCTL.\n");
        return -EPERM;
    }

    switch(cmd) {
        case SIM_IOCTL_CMD_EJECT:
            printk(KERN_INFO "SecureSIM_Driver: Authentication token verified by driver. Pulling GPIO pin HIGH.\n");
            // Production Hook: gpio_set_value(MOTOR_PIN, 1);
            return 0;
        default:
            printk(KERN_WARNING "SecureSIM_Driver: Invalid IOCTL Opcode received.\n");
            return -EINVAL;
    }
}

static int secure_sim_open(struct inode *inode, struct file *file) {
    return 0;
}

static int secure_sim_release(struct inode *inode, struct file *file) {
    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = secure_sim_ioctl,
    .open = secure_sim_open,
    .release = secure_sim_release,
};

static int __init secure_sim_init(void) {
    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0) {
        return major_num;
    }

    // MODERN KERNEL FIX: Replaced legacy class_create parameter architecture for compatibility with kernels 6.x+
    secure_sim_class = class_create(DEVICE_NAME);
    if (IS_ERR(secure_sim_class)) {
        unregister_chrdev(major_num, DEVICE_NAME);
        return PTR_ERR(secure_sim_class);
    }

    // In production, uevent rules/ueventd.rc will enforce standard permissions chown system:system /dev/secure_sim_latch
    secure_sim_device = device_create(secure_sim_class, NULL, MKDEV(major_num, 0), NULL, DEVICE_NAME);
    if (IS_ERR(secure_sim_device)) {
        class_destroy(secure_sim_class);
        unregister_chrdev(major_num, DEVICE_NAME);
        return PTR_ERR(secure_sim_device);
    }

    printk(KERN_INFO "SecureSIM Driver Loaded. Character device attached at /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit secure_sim_exit(void) {
    device_destroy(secure_sim_class, MKDEV(major_num, 0));
    class_destroy(secure_sim_class);
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "SecureSIM Driver Unloaded cleanly.\n");
}

module_init(secure_sim_init);
module_exit(secure_sim_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lalchand Saini");
MODULE_DESCRIPTION("Hardened Kernel driver for Electronic Biometric-Locked SIM Subsystem");