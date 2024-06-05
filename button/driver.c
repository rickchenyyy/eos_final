#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "gpio_matrix"
#define CLASS_NAME "gpio"

// Define the GPIO pins for the rows and columns
static unsigned int row_pins[] = {15, 14};
static unsigned int col_pins[] = {16, 12, 8, 18};

// Device number and class
static int major_number;
static struct class* gpio_class = NULL;
static struct device* gpio_device = NULL;

// File operations: open
static int dev_open(struct inode* inodep, struct file* filep) {
    printk(KERN_INFO "GPIO Matrix driver opened\n");
    return 0;
}

// File operations: write (to set row pins)
static ssize_t dev_write(struct file* filep, const char* buffer, size_t len, loff_t* offset) {
    char input[2];
    if (copy_from_user(input, buffer, sizeof(input))) {
        return -EFAULT;
    }

    gpio_set_value(row_pins[0], input[0]);
    gpio_set_value(row_pins[1], input[1]);
    return len;
}

// File operations: read (to read column pins)
static ssize_t dev_read(struct file* filep, char* buffer, size_t len, loff_t* offset) {
    char col_values[4];
    int i;
    for (i = 0; i < 4; i++) {
        col_values[i] = gpio_get_value(col_pins[i]);
    }

    if (copy_to_user(buffer, col_values, sizeof(col_values))) {
        return -EFAULT;
    }
    return sizeof(col_values);
}

// File operations: release
static int dev_release(struct inode* inodep, struct file* filep) {
    printk(KERN_INFO "GPIO Matrix driver closed\n");
    return 0;
}

// File operations structure
static struct file_operations fops = {
    .open = dev_open,
    .write = dev_write,
    .read = dev_read,
    .release = dev_release,
};

// Module initialization function
static int __init gpio_init(void) {
    int i;

    printk(KERN_INFO "Initializing GPIO Matrix driver\n");

    // Register the device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Failed to register a major number\n");
        return major_number;
    }
    printk(KERN_INFO "Registered with major number %d\n", major_number);

    // Register the device class
    gpio_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gpio_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(gpio_class);
    }
    printk(KERN_INFO "Device class registered\n");

    // Register the device driver
    gpio_device = device_create(gpio_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(gpio_device)) {
        class_destroy(gpio_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(gpio_device);
    }
    printk(KERN_INFO "Device class created\n");

    // Initialize the GPIO pins for rows as output
    for (i = 0; i < 2; i++) {
        if (!gpio_is_valid(row_pins[i])) {
            printk(KERN_INFO "Invalid GPIO %d\n", row_pins[i]);
            return -ENODEV;
        }
        gpio_request(row_pins[i], "sysfs");
        gpio_direction_output(row_pins[i], 0);
        gpio_export(row_pins[i], false);
    }

    // Initialize the GPIO pins for columns as input
    for (i = 0; i < 4; i++) {
        if (!gpio_is_valid(col_pins[i])) {
            printk(KERN_INFO "Invalid GPIO %d\n", col_pins[i]);
            return -ENODEV;
        }
        gpio_request(col_pins[i], "sysfs");
        gpio_direction_input(col_pins[i]);
        gpio_export(col_pins[i], false);
    }

    return 0;
}

// Module exit function
static void __exit gpio_exit(void) {
    int i;
    printk(KERN_INFO "Exiting GPIO Matrix driver\n");

    for (i = 0; i < 2; i++) {
        gpio_unexport(row_pins[i]);
        gpio_free(row_pins[i]);
    }

    for (i = 0; i < 4; i++) {
        gpio_unexport(col_pins[i]);
        gpio_free(col_pins[i]);
    }

    device_destroy(gpio_class, MKDEV(major_number, 0));
    class_unregister(gpio_class);
    class_destroy(gpio_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "GPIO Matrix driver exited\n");
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A GPIO driver for a button matrix on Raspberry Pi");
MODULE_VERSION("0.1");
