/*
 * sched.c schedule a function to be called on every timer interrupt
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

static int TimerIntrpt;
struct proc_dir_entry *Our_Proc_File;
#define PROC_ENTRY_FILENAME "sched"
#define MY_WORK_QUEUE_NAME "WQsched.c"


/*
 * The number of times the timer interrupt has been called so far
*/

static void intrpt_routine(struct work_struct *work);

static int die = 0; /*set this to 1 for shutdown*/

/*
 * the work queue structure for this task
 */

static struct workqueue_struct *my_workqueue;

static struct delayed_work Task;

/*
 * This function will be called on every timer interrupt.
 */
static void intrpt_routine(struct work_struct *work)
{
	/*
	 *Increment the counter
	 */
	TimerIntrpt++;
	if(die == 0)
		queue_delayed_work(my_workqueue, &Task, 100);
}

/*
 * put data inot proc fs file.
 */
static int sched_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%llu\n", (unsigned long long)TimerIntrpt);
	return 0;
}

static int sched_open(struct inode *inode, struct file *file)
{
	return single_open(file, sched_show, NULL);
}
static struct file_operations proc_ops = {
	.owner   = THIS_MODULE,
	.open    = sched_open,	
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
};

static int __init init_mymodule(void)
{
	int rv = 0;
	/*
	 * put the task in the work_timer task queue, so it will be executed at next
	 * timer interrupt
	 */
	INIT_DELAYED_WORK(&Task, intrpt_routine);
	my_workqueue = create_workqueue(MY_WORK_QUEUE_NAME);
	queue_delayed_work(my_workqueue, &Task, 100);

	proc_create_data(PROC_ENTRY_FILENAME, 0644, NULL, &proc_ops, NULL);
	return rv;
}

static void __exit exit_mymodule(void)
{
	/*
	 * unregister our /proc file
	 */
	remove_proc_entry(PROC_ENTRY_FILENAME, NULL);
	printk(KERN_INFO "/proc/%s removed\n", PROC_ENTRY_FILENAME);
	die = 1;
	cancel_delayed_work(&Task);
	flush_workqueue(my_workqueue); /*wait till all 'old ones' finished*/
	destroy_workqueue(my_workqueue);

	/*
	 * sleep untile intrpt_routine is called one last time.
	 * this is necessary
	 */
}

module_init(init_mymodule);
module_exit(exit_mymodule);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is schedule task  module");
MODULE_VERSION("Ver 0.1");
