#define LINUX

#include <linux/module.h>  
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <asm/pgtable.h>
#include <asm/uaccess.h>

#define AUDIT_ENTRY_PML4E if(0)
#define AUDIT_ENTRY_PDPTE if(0)
#define AUDIT_ENTRY_PDE if(0)
#define AUDIT_ENTRY_PTE if(0)

#define AUDIT_RESULT_PML4E if(1)
#define AUDIT_RESULT_PDPTE if(1)
#define AUDIT_RESULT_PDE if(1)
#define AUDIT_RESULT_PTE if(1)

#define AUDIT_ADDRESS if(0)

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

void walk_table(int level, int index_parent, void **table);
void manage_entry(int level, int index,void *entry);

unsigned long long tot_kernel_page = 0;

void manage_entry(int level, int index,void *entry){
	void* control_bit;
        void* address;
        void* real_address_pa;
        void* real_address_va;

	if(level == 3)	return;
	
		
	// Check if 5 bit is 1 -> linear address so this is one page
	if(CHECK_BIT((int)entry,8)!=0 || CHECK_BIT((int)entry,1)==0){
//		printk(KERN_ERR "Level:%d Index:%d KERNEL PAGE\n",level,index);
//		tot_kernel_page++;
		return;
	}

	control_bit = (void *)((ulong) entry & 0x0000000000000fff);
      	address = (void *)((ulong) entry & 0xfffffffffffff000);
     	real_address_pa = address;
      	real_address_va = __va(real_address_pa);
	
	if(!((ulong)control_bit ^ 0x0000000000000061)){ printk(KERN_ERR "L%d I:%d \n",level,index);	return;}

	walk_table(level+1,index,real_address_va);
}

void walk_table(int level, int index_parent, void **table){
        int index;
	int busy;
	int busy_kernel;
	int free;
	int count;
	void* control_bit;
		
	switch(level){
                case 0:
                        count = 512;
                        break;
                case 1:
                        count = 512;
                        break;
                case 2:
                        count = 512;
                        break;
                case 3:
                        count = 512;
                        break;
        }
	
	busy_kernel = 0;
	busy = 0;
	free = 0;

        for(index=0; index<count; index++){
		/*
		if(level == 3 && table[index]!=NULL){
			control_bit = (void *)((ulong) table[index] & 0x0000000000000fff);
			printk(KERN_ERR "PTE %d: \t Control_bit:%p\n",index,control_bit);
		}
		*/
		if(table[index]!=NULL && CHECK_BIT((int)table[index],2)==0) busy_kernel++;
                
		if(table[index]!=NULL) busy++;
		else free++;
        }
	
	switch(level){
		case 0:
			AUDIT_RESULT_PML4E printk(KERN_ERR "[PML4E_BUSY_FROM_KERNEL]: %d\n",busy_kernel);
			AUDIT_RESULT_PML4E printk(KERN_ERR "[PML4E_BUSY]: %d\n",busy);
			AUDIT_RESULT_PML4E printk(KERN_ERR "[PML4E_FREE]: %d\n",free);
			break;
		case 1:	
			AUDIT_RESULT_PDPTE printk(KERN_ERR "\t\t\t (%d)[PDPTE_BUSY_FROM_KERNEL]: %d\n",index_parent,busy_kernel);
			AUDIT_RESULT_PDPTE printk(KERN_ERR "\t\t\t (%d)[PDPTE_BUSY]: %d\n",index_parent,busy);
			AUDIT_RESULT_PDPTE printk(KERN_ERR "\t\t\t (%d)[PDPTE_FREE]: %d\n",index_parent,free);
			break;  
		case 2:
			AUDIT_RESULT_PDE printk(KERN_ERR "\t\t\t\t\t\t (%d)[PDE_BUSY_FROM_KERNEL]: %d\n",index_parent,busy_kernel);
			AUDIT_RESULT_PDE printk(KERN_ERR "\t\t\t\t\t\t (%d)[PDE_BUSY]: %d\n",index_parent,busy);
			AUDIT_RESULT_PDE printk(KERN_ERR "\t\t\t\t\t\t (%d)[PDE_FREE]: %d\n",index_parent,free);
			break;
		case 3:
			if(busy_kernel != 0){
			AUDIT_RESULT_PTE printk(KERN_ERR "\t\t\t\t\t\t\t\t (%d)[PTE_BUSY_FROM_KERNEL]: %d\n",index_parent,busy_kernel);
			AUDIT_RESULT_PTE printk(KERN_ERR "\t\t\t\t\t\t\t\t (%d)[PTE_BUSY]: %d\n",index_parent,busy);
			AUDIT_RESULT_PTE printk(KERN_ERR "\t\t\t\t\t\t\t\t (%d)[PTE_FREE]: %d\n",index_parent,free);
			}
			break;
	}
	
	switch(level){
                case 0:
                        count = 512;
                        break;
                case 1:
                        count = 512;
                        break;
                case 2:
                        count = 512;
                        break;
                case 3:
                        break;
        }
		
	if(level == 3){
                tot_kernel_page = tot_kernel_page + busy_kernel;
		return;
	}
	
	for(index=0; index<count; index++){
		if(table[index]!=NULL){
		/*	if(level != 3 || CHECK_BIT((int)table[index],2)!=0) */manage_entry(level,index,table[index]);
			/*
			else if(CHECK_BIT((int)table[index],2)==0){
				switch(level){
					case 0:
						tot_kernel_page = tot_kernel_page + (512 * 512 * 512);
						break;
					case 1:
						tot_kernel_page = tot_kernel_page + (512*512);
						break;
					case 2:
						tot_kernel_page = tot_kernel_page + 512;
						break;
				}
			}
			*/
		}	
	}

	return;
	
}

int init_module(void){ 
	walk_table(0,0,current->mm->pgd);
	printk(KERN_ERR "RESULT = %llu\n",tot_kernel_page);
	return 0;
}


void cleanup_module(void){}  

MODULE_LICENSE("GPL");

