/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_io.c
 *
 * Description: reset key button  and wifi indicate function realization
 *
 * Modification history:
 *     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "user_io.h"


#include "user_esp_platform.h"
struct esp_platform_saved_param esp_param;
LOCAL struct plug_saved_param plug_param;
LOCAL struct keys_param keys;
LOCAL struct single_key_param *single_key[PLUG_KEY_NUM];
LOCAL os_timer_t link_led_timer;
LOCAL uint8 link_led_level = 0;


uint8 ICACHE_FLASH_ATTR
user_plug_get_status(void)
{
    return plug_param.status;
}

/******************************************************************************
 * FunctionName : user_plug_set_status
 * Description  : set plug's status, 0x00 or 0x01
 * Parameters   : uint8 - status
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_plug_set_status(bool status)
{
    if (status != plug_param.status) {
        if (status > 1) {
            os_printf("error status input!\n");
            return;
        }

        plug_param.status = status;
        PLUG_STATUS_OUTPUT(PLUG_RELAY_LED_IO_NUM, status);
    }
}

void ICACHE_FLASH_ATTR
os_delay_ms(uint16 ms)
{
	uint16 i=1000;
	while(i>0)
	{
	os_delay_us(ms);
	i=i-1;
	}
}

void ICACHE_FLASH_ATTR
wifi_blink(uint8 b_count)
{

	while(b_count>0)
	{
		GPIO_OUTPUT_SET(PLUG_WIFI_LED_IO_NUM, 1);
		os_delay_ms(100);
		GPIO_OUTPUT_SET(PLUG_WIFI_LED_IO_NUM, 0);
		os_delay_ms(100);
		b_count=b_count-1;
	}

}

LOCAL void ICACHE_FLASH_ATTR
user_link_led_timer_cb(void)
{
    link_led_level = (~link_led_level) & 0x01;
    GPIO_OUTPUT_SET(GPIO_ID_PIN(PLUG_WIFI_LED_IO_NUM), link_led_level);
}


void ICACHE_FLASH_ATTR
user_link_led_timer_init(void)
{
    os_timer_disarm(&link_led_timer);
    os_timer_setfn(&link_led_timer, (os_timer_func_t *)user_link_led_timer_cb, NULL);
    os_timer_arm(&link_led_timer, 50, 1);
    link_led_level = 0;
    GPIO_OUTPUT_SET(GPIO_ID_PIN(PLUG_WIFI_LED_IO_NUM), link_led_level);
}

void ICACHE_FLASH_ATTR
user_link_led_timer_done(void)
{
    os_timer_disarm(&link_led_timer);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(PLUG_WIFI_LED_IO_NUM), 0);
}

LOCAL void ICACHE_FLASH_ATTR
user_plug_short_press(void)
{
	user_plug_set_status((~plug_param.status) & 0x01);
	spi_flash_erase_sector(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE);
	spi_flash_write((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
	(uint32 *)&plug_param, sizeof(struct plug_saved_param));
	//os_printf("button short pressed!");
	//uint8 shortpress_flag=0;
}

void ICACHE_FLASH_ATTR
smartconfig_done(sc_status status,void *data)
{
	 switch(status) {
	        case SC_STATUS_WAIT:
	           // os_printf("SC_STATUS_WAIT\n");
	            break;
	        case SC_STATUS_FIND_CHANNEL:
	           // os_printf("SC_STATUS_FIND_CHANNEL\n");
	            break;
	        case SC_STATUS_GETTING_SSID_PSWD:
	           // os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
	            break;
	        case SC_STATUS_LINK:
	        { //   os_printf("SC_STATUS_LINK\n");
	        		struct station_config *sta_conf = data;
	        				wifi_station_set_config(sta_conf);
	        				wifi_station_disconnect();
	        				wifi_station_connect();
	        	            break;}
	        case SC_STATUS_LINK_OVER:
	        {//     os_printf("SC_STATUS_LINK_OVER\n");
	        		user_link_led_timer_done();
	                       uint8 phone_ip[4] = {0};
	                       os_memcpy(phone_ip, (uint8*)data, 4);
	                       os_printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
	smartconfig_stop();
	wifi_station_set_auto_connect(1);
    start();}
	 	 	 	 	 	 	 }
    //os_printf("ap_dhcp_status2:%d\n",wifi_softap_dhcps_status());
}

LOCAL void ICACHE_FLASH_ATTR
user_plug_long_press(void)
{
	//	wifi_blink(15);
    //user_esp_platform_set_active(0);
  // system_restore();
  // system_restart();
	 user_link_led_timer_init();
	 os_printf("start_SmartCnonfig\n");
	 smartconfig_stop();
	//  smartconfig_start(SC_TYPE_ESPTOUCH,smartconfig_done);
	smartconfig_start(smartconfig_done);

}

void ICACHE_FLASH_ATTR
set_plug_status_from_flash(void){

	 spi_flash_read((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
	         		(uint32 *)&plug_param, sizeof(struct plug_saved_param));
	 PLUG_STATUS_OUTPUT(PLUG_RELAY_LED_IO_NUM, plug_param.status);
}

void ICACHE_FLASH_ATTR
user_io_init(void)
{

    wifi_status_led_install(PLUG_WIFI_LED_IO_NUM, PLUG_WIFI_LED_IO_MUX, PLUG_WIFI_LED_IO_FUNC);

    PIN_FUNC_SELECT(PLUG_WIFI_LED_IO_MUX, PLUG_WIFI_LED_IO_FUNC);
    PIN_FUNC_SELECT(PLUG_RELAY_LED_IO_MUX,PLUG_RELAY_LED_IO_FUNC);

    single_key[0] = key_init_single(PLUG_KEY_0_IO_NUM, PLUG_KEY_0_IO_MUX, PLUG_KEY_0_IO_FUNC,
                                    user_plug_long_press, user_plug_short_press);

    keys.key_num = PLUG_KEY_NUM;
    keys.single_key = single_key;
    key_init(&keys);

    spi_flash_read((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
         		(uint32 *)&plug_param, sizeof(struct plug_saved_param));


     // no used SPI Flash
     if (plug_param.status == 0xff) {
         plug_param.status = 0;
     }

   // PLUG_STATUS_OUTPUT(PLUG_RELAY_LED_IO_NUM, plug_param.status);
     PLUG_STATUS_OUTPUT(PLUG_RELAY_LED_IO_NUM, 0);



}


