#include "common.h"

volatile u32 *a11_entry = (volatile u32 *)0x1FFFFFF8;
//volatile u32 *a11_entry_firm = (volatile u32 *)0x1FFFFFFC;


void __attribute__((naked)) a11Entry()
{
    /*__asm__ (
        "CPSID aif\n\t" //Disable interrupts
        "ldr r0,=_stack\n\t"
        "mov sp, r0"
    );*/
__asm__ ("ldr r0,=_stack\n\t mov sp, r0");
    /* Initialize the arm11 thread commands */
    *a11_entry = 0;
    //*a11_entry_firm = 0;

    while (!*a11_entry/*&& !*a11_entry_firm*/);

    /*if(*a11_entry_firm)
        ((void (*)())*a11_entry_firm)(); */
    //else
        ((void (*)())*a11_entry)(); 
    
}
