#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include "config_pages.h"

#define CURRENT_DATA_VERSION    5
#define PAGE_SIZE 64


/**
give access to configPages
*/
extern volatile configPage1_t configPage1;
extern volatile configPage2_t configPage2;
extern volatile configPage3_t configPage3;
extern volatile configPage4_t configPage4;
extern volatile configPage9_t configPage9;
extern volatile configPage10_t configPage10;
extern volatile configPage11_t configPage11;

extern const U16 configPage_size[12];




/**
config handling
*/
U32 write_ConfigData();
U32 load_ConfigData();
U32 migrate_configData();


#endif // CONFIG_H_INCLUDED
