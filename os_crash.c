#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>  // For current process information

void my_panic(const char *msg) {
    printk(KERN_ALERT "Custom Panic: %s\n", msg);
    printk(KERN_ALERT "Process Info: PID=%d, Name=%s\n", current->pid, current->comm);

    // Halt the system by entering an infinite loop with interrupts disabled
    printk(KERN_ALERT "System halting due to custom panic.\n");
    local_irq_disable(); // Disable interrupts on the local CPU
    while (1) {
        cpu_relax();     // Prevent busy looping, allowing low-power halting
    }
}

// Initialization function for module
static int __init panic_module_init(void) {
    printk(KERN_ALERT "Initializing module and triggering custom panic.\n");
    my_panic("Forced custom kernel panic for testing");
    return 0;
}

// Exit function for module
static void __exit panic_module_exit(void) {
    printk(KERN_ALERT "Custom panic module unloaded.\n");
}

module_init(panic_module_init);
module_exit(panic_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shivam Kumar");
MODULE_DESCRIPTION("A module to demonstrate a custom panic function");
