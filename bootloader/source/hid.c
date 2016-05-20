#include "hid.h"

    	
u32 WaitTimeForInput(u32 waittime) {
    for(volatile u64 i=0;i<0xEFF*waittime;i++)
   	{
   	    u32 pad_state = (~HID_STATE)&0xFFF;
        if (pad_state!=0)
           	return pad_state;
    }
    return 0;
}
u32 InputWait() {
    u32 pad_state_old = HID_STATE;
    while (true) {
        u32 pad_state = HID_STATE;
        if (pad_state ^ pad_state_old)
            return ~pad_state;
    }
}
u32 GetInput() { 
    u32 pad_state = HID_STATE; 
    
    return ~pad_state&0xFFF; 
}
