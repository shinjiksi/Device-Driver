/*
	Author: Shinji Kasai
	Class: CS 453
	Project 3
*/

#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/version.h> 
#include <linux/init.h>  
#include <linux/slab.h>
#include <linux/fs.h>     
#include <linux/errno.h>  
#include <linux/types.h>  
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "booga.h"
#include <linux/random.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/signal.h>

static int booga_major =   BOOGA_MAJOR;
static int booga_nr_devs = BOOGA_NR_DEVS;  
module_param(booga_major, int, 0);
module_param(booga_nr_devs, int, 0);
MODULE_AUTHOR("Shinji Kasai");
MODULE_LICENSE("GPL");

char * random_phrases[] = {"booga! booga!", "googoo! gaagaa!","neka! maka!","wooga! wooga!"};
static booga_stats *booga_device_stats;
static struct proc_dir_entry* booga_proc_file;

static ssize_t booga_read (struct file *, char *, size_t , loff_t *);
static ssize_t booga_write (struct file *, const char *, size_t , loff_t *);
static int booga_open (struct inode *, struct file *);
static int booga_release (struct inode *, struct file *);
static int booga_proc_open(struct inode *inode, struct file *file);

static struct file_operations booga_fops = {
		    read:       booga_read,
			write:      booga_write,
			open:       booga_open,
			release:    booga_release,
};

static const struct file_operations booga_proc_fops = {
		.owner	= THIS_MODULE,
		.open	= booga_proc_open,
		.read	= seq_read,
		.llseek	= seq_lseek,
		.release	= single_release,
};

static int booga_open (struct inode *inode, struct file *filp)
{
		int num = NUM(inode->i_rdev);
		booga_device_stats->device_num = num;
		if (num >= booga_nr_devs) return -ENODEV;

		booga_device_stats->numOfOpened[num]++;
		filp->f_op = &booga_fops;

		if (down_interruptible (&booga_device_stats->sem))
				return (-ERESTARTSYS);
		booga_device_stats->num_open++;
		up(&booga_device_stats->sem);

		try_module_get(THIS_MODULE);
		printk(KERN_INFO "Booga: booga open!\n");
		return 0;          /* return when success */
}

static int booga_release (struct inode *inode, struct file *filp)
{
		/* protects if multiple processes invoke the driver 
		to prevent a race condition */
		if (down_interruptible (&booga_device_stats->sem))
				return (-ERESTARTSYS);
		booga_device_stats->num_close++;
		up(&booga_device_stats->sem);

		module_put(THIS_MODULE);
		printk(KERN_INFO "Booga: booga closed!\n");
		return 0;
}

static ssize_t booga_read (struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
		int choice = 0;
		int i = 0;
		int currentChar = 0;
		char* newPhrase = (char*)kmalloc(sizeof(char)*count, GFP_KERNEL);
		char randval;
		char* random_String;

		printk("<1>booga_read invoked.\n");
		/* protects if multiple processes invoke the driver with a semaphore
			to prevent a race condition */
		if (down_interruptible (&booga_device_stats->sem))
				return (-ERESTARTSYS);

		get_random_bytes(&randval, 1);
		choice = (randval & 0x7F) % 4; 

		random_String = random_phrases[choice]; //get random booga string  

		booga_device_stats->numOfStrings[choice]++;
		while(currentChar < count) //write a of the length of the count
		{
			
			if(random_String[i] == '\0') //if reaches to the end of the first random string
				i = 0;	//reset

			newPhrase[currentChar] = random_String[i];


			i++;
			currentChar++;

		}
		
		if(copy_to_user(buf, newPhrase, count)) //copy data
			return (-ERESTARTSYS);

		booga_device_stats->num_read+=count; //increment by the size of count bytes
		up(&booga_device_stats->sem);
		return count; // return when count bytes were read
}

static ssize_t booga_write (struct file *filp, const char *buf, size_t count , loff_t *f_pos)
{
		struct siginfo signal;
		
		printk("<1>booga_write invoked.\n");
		/* protects this with a semaphore if multiple processes invoke driver to prevent a race condition */
		if (down_interruptible (&booga_device_stats->sem))
				return (-ERESTARTSYS);

		if(booga_device_stats->device_num == 3)
		{
			memset(&signal,0,sizeof(struct siginfo));
			signal.si_signo = SIGTERM;
			signal.si_signo = SI_QUEUE;
			signal.si_int = 1;
				
			if (current != NULL)
			{
				printk(KERN_INFO "Sending signal to app\n");
				if(send_sig_info(SIGTERM, &signal, current) < 0)
					printk(KERN_INFO "Unable to send signal\n");
			}
		}
		else
			booga_device_stats->num_write+=count;
		
		up(&booga_device_stats->sem);
		return count; // return when count bytes were written
}

static void init_booga_device_stats(void)
{
		int i = 0;
		booga_device_stats->num_read=0;
		booga_device_stats->num_write=0;
		booga_device_stats->num_open=0;
		booga_device_stats->num_close=0;
		for(i = 0; i < 4; i++)
			booga_device_stats->numOfStrings[i] = 0; //initialize to 0
		for(i = 0; i < 4; i++)
			booga_device_stats->numOfOpened[i] = 0; //initiallize to 0
		
		sema_init(&booga_device_stats->sem, 1);
}

static int booga_proc_show(struct seq_file *m, void *v)
{
		int i = 0;
		seq_printf(m, "bytes read = %ld\n", booga_device_stats->num_read);
		seq_printf(m, "bytes written = %ld\n", booga_device_stats->num_write);
		
		seq_printf(m, "number of opens:\n");
		for(i = 0; i < 4; i++)
			seq_printf(m, "/dev/booga%d = %ld times\n", i, booga_device_stats->numOfOpened[i]);

		seq_printf(m, "strings output:\n");
		for(i = 0; i < 4; i++)
			seq_printf(m, "%s = %ld times\n", random_phrases[i], booga_device_stats->numOfStrings[i]);

		return 0;
}

static int booga_proc_open(struct inode *inode, struct file *file)
{
		return single_open(file, booga_proc_show, NULL);
}

static __init int booga_init(void)
{
		int result;

		/* Register and accept a dynamic number*/
		result = register_chrdev(booga_major, "booga", &booga_fops);
		if (result < 0) {
				printk(KERN_WARNING "booga: can't get major %d\n",booga_major);
				return result;
		}
		if (booga_major == 0) booga_major = result; /* dynamic */
		printk("<1> booga device driver version 4: loaded at major number %d\n", booga_major);

		booga_device_stats = (booga_stats *) kmalloc(sizeof(booga_stats),GFP_KERNEL);
		if (!booga_device_stats) {
				result = -ENOMEM;
				goto fail_malloc;
		}
		init_booga_device_stats();

		booga_proc_file = proc_create("driver/booga", 0, NULL, &booga_proc_fops);
		if (!booga_proc_file)  {
				result = -ENOMEM;
				goto fail_malloc;
		}

		return 0;

fail_malloc:
		unregister_chrdev(booga_major, "booga");
		return  result;
}



static __exit  void booga_cleanup(void)
{
		remove_proc_entry("driver/booga", NULL /* parent dir */);
		kfree(booga_device_stats);
		unregister_chrdev(booga_major, "booga");
		printk("<1> booga device driver version 4: unloaded\n");
}


module_init(booga_init);
module_exit(booga_cleanup);

/* vim: set ts=4: */