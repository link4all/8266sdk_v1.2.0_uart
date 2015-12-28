#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "power_level.h"
#include "user_io.h"

//LOCAL struct plug_saved_param plug_param;

void ICACHE_FLASH_ATTR
 get_power_level(void)
{
	uint8 cmd[8];
	//cmd=(uint8 *)os_malloc(8*sizeof(uint8));
	cmd[0]=0xfe;
	cmd[1]=0x11;
	cmd[2]=0x19;
	cmd[3]=0x0;
	//*(cmd+4)=0;
	cmd[4]=0;
	cmd[5]=0;
	cmd[6]=0;
	cmd[7]=(~(cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]))+0x33;
	uart0_tx_buffer(cmd,8);
	//os_delay_us(50000);
   // return plug_param.power_level;
	//os_free(cmd);
}

void ICACHE_FLASH_ATTR
init_V9250(void){
	uint8 cmd[8];
	cmd[0]=0xfe;
	cmd[1]=0x12;
	cmd[2]=0x87;
	cmd[3]=0x00;
	cmd[4]=0x60;
	cmd[5]=0x00;
	cmd[6]=0x00;
	cmd[7]=(~(cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]))+0x33;
	uart0_tx_buffer(cmd,8);
	os_delay_us(60000);
	os_delay_us(60000);
	cmd[2]=0x85;
	cmd[3]=0x81;
	cmd[4]=0x17;
	cmd[5]=0x00;
	cmd[6]=0x1d;
	cmd[7]=(~(cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]))+0x33;
	uart0_tx_buffer(cmd,8);
	os_delay_us(60000);
	os_delay_us(60000);
	cmd[2]=0x84;
	cmd[3]=0x00;
	cmd[4]=0x00;
	cmd[5]=0x0b;
	cmd[6]=0x0a;
	cmd[7]=(~(cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]))+0x33;
	uart0_tx_buffer(cmd,8);
	os_delay_us(60000);
	os_delay_us(60000);
	cmd[2]=0x83;
	cmd[3]=0x00;
	cmd[4]=0x80;
	cmd[5]=0x03;
	cmd[6]=0x00;
	cmd[7]=(~(cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]))+0x33;
	uart0_tx_buffer(cmd,8);
	os_delay_us(60000);
	os_delay_us(60000);
	cmd[2]=0x80;
	cmd[3]=0x04;
	cmd[4]=0x00;
	cmd[5]=0xfb;
	cmd[6]=0x01;
	cmd[7]=(~(cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]))+0x33;
	uart0_tx_buffer(cmd,8);
	os_delay_us(60000);
	os_delay_us(60000);
	cmd[2]=0x25;
	cmd[3]=0xa7;
	cmd[4]=0x2b;
	cmd[5]=0x1d;
	cmd[6]=0x81;
	cmd[7]=(~(cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]))+0x33;
	uart0_tx_buffer(cmd,8);
	os_delay_us(60000);
	os_delay_us(60000);
	cmd[2]=0x32;
	cmd[3]=0x60;
	cmd[4]=0x9e;
	cmd[5]=0x06;
	cmd[6]=0xfa;
	cmd[7]=(~(cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]))+0x33;
	uart0_tx_buffer(cmd,8);
	os_delay_us(60000);
	os_delay_us(60000);
	cmd[2]=0x2c;
	cmd[3]=0x80;
	cmd[4]=0xb2;
	cmd[5]=0xeb;
	cmd[6]=0x10;
	cmd[7]=(~(cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]))+0x33;
	uart0_tx_buffer(cmd,8);
	os_delay_us(60000);
	os_delay_us(60000);
	cmd[2]=0x2e;
	cmd[3]=0xd0;
	cmd[4]=0x22;
	cmd[5]=0x29;
	cmd[6]=0x09;
	cmd[7]=(~(cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]))+0x33;
	uart0_tx_buffer(cmd,8);


}
