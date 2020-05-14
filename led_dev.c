#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>

#include <linux/kernel.h>
#include <linux/major.h>

#include <linux/proc_fs.h>

#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>

#include <linux/ide.h>

#include <linux/types.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

//hardware maps
#define CCM_CCGR1_BASE (0X020C406C)
#define SW_MUX_GPIO5_IO03_BASE (0X02290014)
#define SW_PAD_GPIO5_IO03_BASE (0X02290058)
#define GPIO5_DR_BASE (0X020AC000)
#define GPIO5_GDIR_BASE (0X020AC004)

#define LED_ON 1
#define LED_OFF 0

static void __iomem * IMX6U_CCM_CCGR1;
static void __iomem * SW_MUX_GPIO5_IO03;
static void __iomem * SW_PAD_GPIO5_IO03;
static void __iomem * GPIO5_DR;
static void __iomem * GPIO5_GDIR;

//hardware function
void led_opr(u8 opr)
{
	u32 tmp;
	if(opr == LED_ON){
	tmp = readl(GPIO5_DR);
	tmp &= ~(1<<3); 
	writel(tmp, GPIO5_DR);
	}
	else if(opr == LED_OFF){
	tmp = readl(GPIO5_DR);
	tmp |= (1<<3); 
	writel(tmp, GPIO5_DR);
	}
}

//set main dev code
static int major = 200;
static char dev_buf[100];

//read write open release func
static int led_open(struct inode *inode, struct file *file)
{
	printk("chardev led opened.\r\n");
	return 0;
}


static ssize_t led_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{

	printk("read ok.\r\n");
	return 0;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err = 0;
	err = copy_from_user(dev_buf, buf, size);
	printk("led operating.\r\n");
	
	if(dev_buf[0] == LED_ON){
	led_opr(LED_ON);
	}
	else if(dev_buf[0] == LED_OFF){
	led_opr(LED_OFF);
	}
	
	return 0;
}

static int led_release(struct inode *inode, struct file *file)
{
	printk("chardev led released.\r\n");
	return 0;
}


//struct file opr
static struct file_operations led_dev = {
	.owner = THIS_MODULE,
	.open=  led_open,
	.read = led_read,
	.write = led_write,
	.release = led_release,
};

//init and exit
static int __init led_init(void)
{
	int err = 0;
	u32 tmp = 0;
	
	printk("ready to init led dev.\r\n");
    //map register
	IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
	SW_MUX_GPIO5_IO03 = ioremap(SW_MUX_GPIO5_IO03_BASE, 4);
	SW_PAD_GPIO5_IO03 = ioremap(SW_PAD_GPIO5_IO03_BASE, 4);
	GPIO5_DR = ioremap(GPIO5_DR_BASE, 4);
	GPIO5_GDIR = ioremap(GPIO5_GDIR_BASE, 4);

	//enable clock of gpio5
	tmp = readl(IMX6U_CCM_CCGR1);
	tmp |= 0xffff0000;
	writel(tmp, IMX6U_CCM_CCGR1);
	//set mux
	writel( 5, SW_MUX_GPIO5_IO03);
	//set pad
	writel( 0x10b0, SW_PAD_GPIO5_IO03);	
	//set gpio as output
	tmp = readl(GPIO5_GDIR);
	tmp |= 0x08;
	writel(tmp, GPIO5_GDIR);	
	//set default val=1
	tmp = readl(GPIO5_DR);
	tmp |= (1<<3); 
	writel(tmp, GPIO5_DR);
	
	err = register_chrdev(major, "led_dev", &led_dev);

	return 0;
}

static void __exit led_exit(void)
{
    //release iomap
	iounmap(IMX6U_CCM_CCGR1);
	iounmap(SW_MUX_GPIO5_IO03);
	iounmap(SW_PAD_GPIO5_IO03);
	iounmap(GPIO5_DR);
	iounmap(GPIO5_GDIR);

	unregister_chrdev(major, "led_dev");
	printk("led dev unregisted.\r\n");
}

module_init(led_init);
module_exit(led_exit);

//appendix
MODULE_LICENSE("GPL");
MODULE_AUTHOR("XJJ");


