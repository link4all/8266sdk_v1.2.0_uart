/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "driver/uart.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "espconn.h"
#include "mem.h"
#include "user_config.h"

#include "power_level.h"
#include "user_io.h"

LOCAL struct plug_saved_param plug_param;
ip_addr_t esp_server_ip;
ip_addr_t esp_server_ip2;

LOCAL os_timer_t auth_msg;
LOCAL os_timer_t heart_beat;
LOCAL os_timer_t con;
LOCAL os_timer_t smartconfig_timer;
LOCAL os_timer_t test_timer;
//LOCAL os_timer_t udp_send_timer;
LOCAL os_timer_t client_timer;
//LOCAL os_timer_t client_timer2;
//LOCAL os_timer_t tcp_send_timer;

LOCAL os_timer_t power_timer;

//LOCAL  uint8 *udpStr=NULL;
LOCAL struct espconn esp_conn;
LOCAL struct espconn esp_conn2;
LOCAL esp_tcp esptcp;
LOCAL esp_tcp esptcp2;
LOCAL struct espconn user_udp_espconn;
//LOCAL struct espconn user_udp_espconn2;
//uint8 *macaddr;
//uint8 *ip_relay;
//uint8  *msg;
struct ip_addr ip_addr2;
 LOCAL uint32 power_count=0;
 LOCAL uint32 power_buffer=0;
 bool ref_flag=true;
 uint32 ref_power=0;

 //uint32 power_balance;
 char ip_mac[30];
 uint8 msg[17];
 uint8 macaddr[6];

 uint8 flag=1;


void ICACHE_FLASH_ATTR TCPconnErr(void *arg, sint8 err);

void ICACHE_FLASH_ATTR
user_udp_send(uint8 *Buffer,int len)
{
    espconn_sent(&user_udp_espconn, Buffer, len);
 }

//void ICACHE_FLASH_ATTR
//udp_sends()
//{
//	os_timer_disarm(&udp_send_timer);
	//os_printf("udpData:%s,len:%d\n",udpStr,os_strlen(udpStr));
//	user_udp_send(udpStr,os_strlen(udpStr));
//}

/******************************************************************************
 * FunctionName : user_udp_recv_cb
 * Description  : Processing the received udp packet
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/

LOCAL void ICACHE_FLASH_ATTR
user_udp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	         uint16 crc=0;

	       //if(msg==NULL) msg = (uint8 *)os_malloc(17);
			 msg[0]= 0xaa;
			 msg[1]= 0xaa; //0xaa0xaa start code
			 msg[4]= macaddr[0];
			 msg[5]= macaddr[1];
			 msg[6]= macaddr[2];
			 msg[7]= macaddr[3];
			 msg[8]= macaddr[4];
			 msg[9]= macaddr[5];  // 6 bytesmacaddr

	    //return mac address and ip
			 uint8 len;
			 for(len=0;len<length;len++)
			 {
	    os_printf("%02x", *(pusrdata+len));
			 }
	   	char *recv2="HF-A31ASSISTHREAD";
	    	if(os_strncmp(pusrdata,recv2,17)==0)
	    {

	    	uint8 relay_statu=(user_plug_get_status()>0)?1:0;
	    	//if(ip_mac==NULL) ip_mac= (char *)os_malloc(30*sizeof(char));
	    	struct ip_info *ipinfo2;
	    	ipinfo2=(struct ip_info *)os_zalloc(sizeof(struct ip_info));
	    	wifi_get_ip_info(STATION_IF, ipinfo2);
	     	wifi_get_macaddr(STATION_IF, macaddr);
	        os_sprintf(ip_mac,"%d.%d.%d.%d,%02X%02X%02X%02X%02X%02X,%d\n",
	        		ipinfo2->ip.addr&0xFF,ipinfo2->ip.addr>>8&0xFF,ipinfo2->ip.addr>>16&0xFF,ipinfo2->ip.addr>>24&0xFF,
					macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5],relay_statu);
	    	espconn_sent(&user_udp_espconn, ip_mac, os_strlen(ip_mac));
	    	//os_delay_ms(2000);
	    	//os_free(ip_mac);
	    }
   //


    if (*pusrdata==0xaa && *(pusrdata+1)==0xaa && *(pusrdata+4)==macaddr[0]
    && *(pusrdata+5)==macaddr[1] && *(pusrdata+6)==macaddr[2] && *(pusrdata+7)==macaddr[3]
	&& *(pusrdata+8)==macaddr[4] && *(pusrdata+9)==macaddr[5])
    {
         switch (*(pusrdata+10))
         {
         case 0x01 : {
			 msg[2]=0x0d;
			 msg[3]=0x0;  //0x010x0c total 12bytes
			 msg[10]= 0x01;       //0x02 heart beat operate code
			 msg[11]= (user_plug_get_status()>0)?1:0;
        	 uint8 i;
        	 for(i=0;i<12;i++){
        		crc=crc+msg[i];
        	 }
          // crc = (0xaa+0xaa+0x0d+(*macaddr)+(*(macaddr+1))+(*(macaddr+2))+(*(macaddr+3))+(*(macaddr+4))+(*(macaddr+5))+0x01+*(msg+11)); //crc code
        	 msg[12]= (uint8)crc;
        	 espconn_sent(&user_udp_espconn, msg, 13);
        	 espconn_sent(&esp_conn, msg, 13);
             }  break;

         case 0x03 :  {

        	 	msg[2]=0x0d;
        	 	msg[3]=0x0;  //0x010x0c total 12bytes
        	 	msg[10]= 0x03;       //0x02 heart beat operate code
        	// 	msg[11]= (user_plug_get_status()>0)?1:0;
        	 	msg[11]= *(pusrdata+11);
        	 	uint8 i;
        	 for(i=0;i<12;i++){
        		crc=crc+msg[i];
        	}
        	 //crc= (0xaa+0xaa+0x0d+(*macaddr)+(*(macaddr+1))+(*(macaddr+2))+(*(macaddr+3))+(*(macaddr+4))+(*(macaddr+5))+0x03+*(msg+11)); //crc code
        	 msg[12]= (uint8)crc;

        	 if(1 == flag ){
        	 espconn_sent(&user_udp_espconn, msg, 13);
        	 espconn_sent(&esp_conn, msg, 13);

        	user_plug_set_status(*(pusrdata+11));
        		    plug_param.status=(*(pusrdata+11));
        		    spi_flash_erase_sector(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE);
        		    spi_flash_write((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
        		    (uint32 *)&plug_param, sizeof(struct plug_saved_param));
        	 	 	 	}
        	 flag=0;
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(50000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		os_delay_us(60000);
    		flag=1;


             }  break;
         case 0x04 :  {

        	 msg[2]=0x10;
        	 msg[3]=0x0;  //0x010x0c total 12bytes
        	 msg[10]= 0x04;       //0x02 heart beat operate code
        	 msg[11]= power_count&0xFF;
        	 msg[12]= (power_count>>8)&0xFF;
        	 msg[13]= (power_count>>16)&0xFF;
        	 msg[14]= (power_count>>24)&0xFF;
        	 uint8 i;
        	 for(i=0;i<15;i++){
        		crc=crc+msg[i];
        	 }
        	 msg[15]= (uint8)crc;
        	         	 espconn_sent(&user_udp_espconn, msg, 16);
        	         	 espconn_sent(&esp_conn, msg, 16);
             }  break;
      default : ;

       }
    }
    //os_delay_ms(2000);
    //os_free(msg);
}



LOCAL void ICACHE_FLASH_ATTR
send_auth_msg(void){
	 char msg2[87];
	//if(msg2==NULL) msg2 = (char *)os_malloc(sizeof(char)*(87));

	 os_sprintf(msg2,"GET /8266.php?mac=%02X%02X%02X%02X%02X%02X HTTP/1.0\r\nHOST: 50.118.22.184:80\r\nConnection: Close\r\n\r\n",macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
//	 os_printf("%s",msg2);
/*******************************************************
	 char *get="GET /8266.php HTTP/1.0\r\n";
	 char *host="HOST: 50.118.22.184:80\r\n";
	 char *end="\r\n\r\n";
	espconn_sent(&esp_conn2, get,24);
	os_printf("%s",get);
	espconn_sent(&esp_conn2, host,24);
	os_printf("%s",host);
	espconn_sent(&esp_conn2, end,4);
	os_printf("%s",end);
	********************************************/
	// os_printf("%s----%d chars\n",msg2,os_strlen(msg2));
	os_printf("auth msg sending .....\n");
	espconn_sent(&esp_conn2, msg2,os_strlen(msg2));
	//os_delay_ms(2000);
	//os_free(msg2);

}

/*****************************************************************************
     * FunctionName : user_udp_sent_cb
     * Description  : udp sent successfully
     * Parameters  : arg -- Additional argument to pass to the callback function
     * Returns      : none
*******************************************************************************/
 LOCAL void ICACHE_FLASH_ATTR
 user_udp_sent_cb(void *arg)
 {
     struct espconn *pespconn = arg;
          //os_printf("user_udp_send successfully !!!\n");
    //if(ip_mac!=NULL) os_free(ip_mac);

 }


void ICACHE_FLASH_ATTR
udp_check_ip(void)
{
    struct ip_info ipconfig;
    os_timer_disarm(&test_timer);
   //get ip info of ESP8266 station
    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0)
   {
      //os_printf("udp got ip !!! \r\n");
      //wifi_set_broadcast_if(STATIONAP_MODE); // send UDP broadcast from both station and soft-AP interface
      wifi_set_broadcast_if(STATION_MODE);

      user_udp_espconn.type = ESPCONN_UDP;
      user_udp_espconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
      user_udp_espconn.proto.udp->local_port = UDPORT; // set a available  port

      const char udp_remote_ip[4] = {255, 255, 255, 255};
      os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4); // ESP8266 udp remote IP

      espconn_regist_recvcb(&user_udp_espconn, user_udp_recv_cb); // register a udp packet receiving callback
      espconn_regist_sentcb(&user_udp_espconn, user_udp_sent_cb); // register a udp packet sent callback
      espconn_create(&user_udp_espconn);   // create udp


    }
   else
   {
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL))
        {
         os_printf("udp_connect fail !!! \r\n");
        }
      else
      {
           //re-arm timer to check ip
            os_timer_setfn(&test_timer, (os_timer_func_t *)udp_check_ip, NULL);
            os_timer_arm(&test_timer, 100, 0);
        }
    }


}



void ICACHE_FLASH_ATTR
user_tcp_send(uint8 *str,int len)
{
	os_printf("%s\n",str);
	//if(udpStr==NULL) udpStr = (uint8 *)os_malloc(sizeof(uint8)*(len+1));
	// os_memcpy(udpStr,str,len+1);
	 espconn_sent(&esp_conn,str,len+1);
	 //os_delay_ms(2000);
	 //os_free(udpStr);
	//os_timer_disarm(&tcp_send_timer);
	//os_timer_setfn(&tcp_send_timer, (os_timer_func_t *)udp_sends, NULL); // only send next packet after prev packet sent successfully
	//os_timer_arm(&tcp_send_timer, 10000, 0);
}


LOCAL void ICACHE_FLASH_ATTR
user_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

    if (ipaddr == NULL) {
        os_printf("user__dns_found NULL\n");
        return;
    }
  //  os_printf("user__dns_found %d.%d.%d.%d\n",
 //           *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
//          *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

    if (esp_server_ip.addr == 0 && ipaddr->addr != 0) {
        os_timer_disarm(&client_timer);
        esp_server_ip.addr = ipaddr->addr;
        os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);
    }
}




LOCAL void ICACHE_FLASH_ATTR
user__dns_check_cb(void *arg)
{
    struct espconn *pespconn = arg;

    os_printf("user_dns_check_cb\n");

    espconn_gethostbyname(pespconn, DOMAIN, &esp_server_ip, user_dns_found);

    os_timer_arm(&client_timer, 1000, 0);
}



LOCAL void ICACHE_FLASH_ATTR
user_start_dns(struct espconn *pespconn)
{
    esp_server_ip.addr = 0;
    espconn_gethostbyname(pespconn, DOMAIN, &esp_server_ip, user_dns_found);

    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user__dns_check_cb, pespconn);
    os_timer_arm(&client_timer, 1000, 0);

}



LOCAL void ICACHE_FLASH_ATTR
TcpRecv_cb2(void *arg2, char *pusrdata2, unsigned short length2)
{

	os_printf("%s",pusrdata2); //闁告绮敮锟絫cp_rec_date闁挎稒鑹捐ぐ澶愬箣閹邦剛鏆氶柛蹇嬪姂閿熻棄绻嬬槐锟�
	char str1[18];
	//if(str1==NULL) str1=(char *)os_malloc(sizeof(char)*(18));

	char auth_str[20];
	//if(auth_str==NULL) auth_str=(char *)os_malloc(sizeof(char)*(20));
	//auth_str="notnotauthednotnot";
	os_strcpy(auth_str,"notnotauthednotnot");
	os_strncpy(str1,auth_str,15);

	os_printf("\n%s\n",os_strstr(pusrdata2,str1));

	if (os_strstr(pusrdata2,str1)) {
		os_printf("firmware not authed , will restart\n");
		system_restart();

	}
	else{
		os_printf("firmware authed\n");
	}
	//os_free(str1);
	//os_free(auth_str);
}


LOCAL void ICACHE_FLASH_ATTR
TcpSent_cb(void *arg)
{
    struct espconn *pespconn = arg;
    os_printf("data send OK!\n");
}

LOCAL void ICACHE_FLASH_ATTR
registTcpRecSend_cb(void *arg)
{
    struct espconn *pesp_conn = arg;
    espconn_regist_recvcb(pesp_conn, user_udp_recv_cb);
    espconn_regist_sentcb(pesp_conn, TcpSent_cb);
    os_printf("connected to netseedcn.com\n");
}


LOCAL void ICACHE_FLASH_ATTR
registTcpRecSend_cb2(void *arg)
{
    struct espconn *pesp_conn = arg;
    espconn_regist_recvcb(pesp_conn, TcpRecv_cb2);
    espconn_regist_sentcb(pesp_conn, TcpSent_cb);
    os_printf("connected to link-4all.com\n");
}



void ICACHE_FLASH_ATTR
tcpConnect(uint32 port,uint32 port2)
{

  //const char esp_server_ip[4] = {61,145,163,34}; //netseed ����ip
	const char esp_server_ip[4] = {52,5,229,128}; //netseed ����ip
 // const char esp_server_ip[4] = {192,168,10,5};
    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = 8899;
    esp_conn.proto.tcp->remote_port = port;
    os_memcpy(esp_conn.proto.tcp->remote_ip, esp_server_ip, 4);//婵炶揪缍�濞夋洟寮ˇ濡宻,濠电偛顦崝鎴﹀闯閿燂拷
  //  user_start_dns(&esp_conn);
 //   os_printf("connect to %d.%d.%d.%d",esp_conn.proto.tcp->remote_ip[0],esp_conn.proto.tcp->remote_ip[1],esp_conn.proto.tcp->remote_ip[2],esp_conn.proto.tcp->remote_ip[3]);



    const char esp_server_ip2[4] = {50,118,22,184};
    	esp_conn2.type = ESPCONN_TCP;
        esp_conn2.state = ESPCONN_NONE;
        esp_conn2.proto.tcp = &esptcp2;
    //   user_start_dns2(&esp_conn2);
        esp_conn2.proto.tcp->local_port = 8090;
        esp_conn2.proto.tcp->remote_port = port2;
        os_memcpy(esp_conn2.proto.tcp->remote_ip, esp_server_ip2, 4);

    espconn_regist_connectcb(&esp_conn, registTcpRecSend_cb);
    espconn_regist_reconcb(&esp_conn, TCPconnErr);
    espconn_connect(&esp_conn);

    espconn_regist_connectcb(&esp_conn2, registTcpRecSend_cb2);
    espconn_regist_reconcb(&esp_conn2, TCPconnErr);
    espconn_connect(&esp_conn2);

}



void ICACHE_FLASH_ATTR
tcp_check_ip(void)
{

	//os_printf("tcp_check_ip started!\n");
	struct ip_info ipconfig;
	    wifi_get_ip_info(STATION_IF, &ipconfig);
	 if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
	  //  os_printf("max:%d",espconn_tcp_get_max_con());
	    tcpConnect(TCPORT,TCPORT2);
	   // tcpConnect(TCPORT2);
	   // os_delay_ms(3000);
	    //send_auth_msg();
	 }
	 else
	 {
		 os_timer_disarm(&con);
		 os_timer_setfn(&con, (os_timer_func_t *)tcp_check_ip, NULL);
		 os_timer_arm(&con, 1000, 0);
	 }


}


void ICACHE_FLASH_ATTR
tcpCon()
{
	tcpConnect(TCPORT,TCPORT2);
}



void ICACHE_FLASH_ATTR
TCPconnErr(void *arg, sint8 err){
	//os_printf("TCPconnect err\n");
	os_timer_disarm(&con);
	os_timer_setfn(&con, (os_timer_func_t *)tcp_check_ip, NULL);
	os_timer_arm(&con, 3000, 0);
}


LOCAL void ICACHE_FLASH_ATTR
send_heart_beat(){
	uint16 crc=0;
	//if(msg==NULL)  msg = (uint8 *)os_malloc(17);
		 msg[0]=0xaa;
		 msg[1]=0xaa; //0xaa0xaa start code
		 msg[2]=0x11;
		 msg[3]=0x0;  //0x010x0c total 12bytes
		 msg[4]= macaddr[0];
		 msg[5]= macaddr[1];
		 msg[6]= macaddr[2];
		 msg[7]= macaddr[3];
		 msg[8]= macaddr[4];
		 msg[9]= macaddr[5];  // 6 bytesmacaddr
		 msg[10]= 0x02;       //0x02 heart beat operate code
		 msg[11]= power_count&0xFF;
		 msg[12]= (power_count>>8)&0xFF;
		 msg[13]= (power_count>>16)&0xFF;
		 msg[14]= (power_count>>24)&0xFF;
		 msg[15]= (user_plug_get_status()>0)?1:0;
    	 uint8 i;
    	 for(i=0;i<16;i++){
    		crc=crc+msg[i];
    	 }

    	 msg[16]= (uint8)crc;
		 //crc = (0xaa+0xaa+(*macaddr)+(*(macaddr+1))+(*(macaddr+2))+(*(macaddr+3))+(*(macaddr+4))+(*(macaddr+5))+0x02+0x0c); //crc code
		 os_printf("heart beat sending .....\n");

		 if (wifi_station_get_connect_status() != STATION_GOT_IP){
		 		 wifi_station_set_auto_connect(1);
		 	 }

		 tcpConnect(TCPORT,TCPORT2);

		 espconn_sent(&esp_conn, msg, 17);
		 espconn_sent(&user_udp_espconn,msg,17);
		// os_delay_ms(2000);
		// os_free(msg);
}

void ICACHE_FLASH_ATTR
save_power_level(uint8 *str){
	uint8 power_crc=0;
	power_crc=(uint8)((~(*str+*(str+1)+*(str+2)+*(str+3)+*(str+4)+*(str+5)+*(str+6)))+0x33);
	if(ref_flag){
		if (*(str+6)==0xff){
		ref_power=(0-(*(str+3)+((*(str+4))<<8)+(0xff<<16)+(0xff<<24)))*6;
		}
		if  (*(str+6)==0x0){
			ref_power=(*(str+3)+((*(str+4))<<8)+((*(str+5))<<16)+((*(str+6))<<24))*4;
		}
	}
	else{
	if(*str==0xfe && *(str+1)==0x11 && *(str+7)==power_crc ){
		uint32 power_buffer_temp;
		power_buffer_temp=*(str+3)+((*(str+4))<<8)+((*(str+5))<<16)+((*(str+6))<<24);
		if((power_buffer_temp>ref_power) && (*(str+6)!=0xff)){
			if((power_buffer_temp+power_buffer)<0x6533F){
				 power_buffer=power_buffer_temp+power_buffer;
			     }
			 if((power_buffer_temp+power_buffer)>=0x6533F){
				 power_count++;
				 power_buffer=power_buffer_temp+power_buffer-0x6533F;
			     }
									}
	   }
	}
}

void ICACHE_FLASH_ATTR
start(){
	os_timer_disarm(&con);
    os_timer_setfn(&con, (os_timer_func_t *)tcp_check_ip, NULL);
    os_timer_arm(&con, 1500, 0);

    os_timer_disarm(&test_timer);
    os_timer_setfn(&test_timer, (os_timer_func_t *)udp_check_ip, NULL);
    os_timer_arm(&test_timer, 1500, 0);

	os_timer_disarm(&heart_beat);
    os_timer_setfn(&heart_beat, (os_timer_func_t *)send_heart_beat, NULL);
    os_timer_arm(&heart_beat, 15000, 1);

	os_timer_disarm(&auth_msg);
    os_timer_setfn(&auth_msg, (os_timer_func_t *)send_auth_msg, NULL);
    os_timer_arm(&auth_msg, 20000,0);

	struct ip_info *ipinfo2;
    ipinfo2=(struct ip_info *)os_zalloc(sizeof(struct ip_info));
    wifi_get_ip_info(STATION_IF, ipinfo2);
    ip_addr2 = ipinfo2->ip;

	os_timer_disarm(&power_timer);
    os_timer_setfn(&power_timer, (os_timer_func_t *)get_power_level, NULL);
    os_timer_arm(&power_timer, 1280,1);

    os_delay_us(60000);
    os_delay_us(60000);
    os_delay_us(60000);
    ref_flag=false;

	//if(macaddr==NULL) macaddr = (uint8 *)os_malloc(6*sizeof(uint8));
	wifi_get_macaddr(STATION_IF, macaddr);
	os_printf("My mac is:%02X%02X%02X%02X%02X%02X\n", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	//os_free(macaddr);
//	if(ref_power!=0){
	set_plug_status_from_flash();
//	}

}



void ICACHE_FLASH_ATTR
isSmartConfig()
{
	    os_printf("Wifi status is: %d\n",wifi_station_get_connect_status());

	   //if (wifi_station_get_connect_status() == STATION_GOT_IP ||wifi_station_get_connect_status() == STATION_CONNECTING)
		   if (wifi_station_get_connect_status() == STATION_GOT_IP)
		 //if (wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
		 //||wifi_station_get_connect_status() ==STATION_CONNECT_FAIL
		 {

		   os_printf("Got ssid and password config,enter directly!\n");
	  	   start();
	  	 os_timer_disarm(&smartconfig_timer);
		 }

	   else
	   {
	/********************************************************************************
		   user_link_led_timer_init();
		   os_printf("start_SmartCnonfig\n");
		 //  smartconfig_start(SC_TYPE_ESPTOUCH,smartconfig_done);
		   smartconfig_start(smartconfig_done); //smartconfigV2.4 no need SC_TYPE_ESPTOUCH any more
   ***********************************************************************************/
		   wifi_station_set_auto_connect(1);
	   }
}


void user_rf_pre_init(void)
{
}

void user_init(void)
{
	 // system_restore();
	   wifi_set_opmode(STATION_MODE);
	   wifi_station_set_auto_connect(1);

	   uart_init(BIT_RATE_2400,BIT_RATE_115200);
	   //uart0_tx_buffer(uint8 *buf, uint16 len);
	   os_printf("SDK version:%s\n", system_get_sdk_version());
	   user_io_init();

		os_delay_us(60000);
		os_delay_us(60000);
		os_delay_us(60000);
		os_delay_us(60000);
	    init_V9250();

	   os_timer_disarm(&smartconfig_timer);
	   os_timer_setfn(&smartconfig_timer, (os_timer_func_t *)isSmartConfig, NULL);
	   os_timer_arm(&smartconfig_timer, 10000, 1);
	//   isSmartConfig();
}

