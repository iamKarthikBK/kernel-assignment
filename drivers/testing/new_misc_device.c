// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/errno.h>

#define LFID "PES2201800185"
#define LFID_LENGTH 14

MODULE_LICENSE("GPL");
MODULE_AUTHOR("B K Karthik");
MODULE_DESCRIPTION("Kernel threading with a misc char device");

DECLARE_WAIT_QUEUE_HEAD(wee_wait);

static struct task_struct  * my_thread;

static int thread_init(void *data)
{
	while (true) {
		if (wait_event_interruptible(wee_wait, kthread_should_stop()))
			return -ERESTARTSYS;

		if (kthread_should_stop())
			break;
	}

	return 0;
}

static ssize_t my_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos)
{
	char *print_str = LFID;
	char input[LFID_LENGTH];
	int len = LFID_LENGTH;
	ssize_t retval = -EINVAL;

	if (count != len)
		return retval;

	retval = simple_write_to_buffer(input, count, ppos, buff, count);

	if (retval < 0)
		return retval;

	retval = strncmp(print_str, input, count) ? count : -EINVAL;
	return retval;
}

static const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.write = my_write
};

static struct miscdevice my_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "pesuio",
	.fops = &my_fops,
	.mode = 0222
};

static int __init my_init(void)
{
	char *eudy_thread = "pesuio";

	my_thread = kthread_run(thread_init, NULL, eudy_thread);
	if (my_thread == ERR_PTR(-ENOMEM))
		return -ENOMEM;

	return misc_register(&my_dev);
}

static void __exit my_exit(void)
{
	kthread_stop(my_thread);
	misc_deregister(&my_dev);
}

module_init(my_init);
module_exit(my_exit);
