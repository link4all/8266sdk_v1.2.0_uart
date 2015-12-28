#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define DOMAIN      "www.netseedcn.com"
//LOCAL struct espconn *conn;


#define TCPORT 38899
#define UDPORT 18899
//#define UDPORT2 48899

#define AUTHDOMAIN      "www.link-4all.com"
#define TCPORT2 80

#define USE_DNS

#ifdef USE_DNS
#define DOMAIN      "www.netseedcn.com"
#endif

#define PASSWORD	"12345678"

#define AP_CACHE           1

#if AP_CACHE
#define AP_CACHE_NUMBER    5
#endif

#endif


