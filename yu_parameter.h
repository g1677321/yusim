#ifndef YU_PARAMETER_H
#define YU_PARAMETER_H

	/*SIMULATOR*/
	#define DISKSIM_SECTOR   512 //(bytes)
	#define SSD_PAGE_SIZE    4096 //(bytes)
	#define SSD_PAGE2SECTOR (SSD_PAGE_SIZE/DISKSIM_SECTOR)
	#define SSD_PAGES_PER_BLOCK 64
	#define SSD_BLOCK_SIZE   (SSD_PAGE_SIZE*SSD_PAGES_PER_BLOCK) //(bytes)
	//#define SSD_BLOCK2SECTOR (SSD_BLOCK_SIZE/DISKSIM_SECTOR)

	#define TIME_PERIOD 1000 //VSSD uses 1000.0
	#define NUM_OF_USER 1 //Hint:trace的userno是由1至n
	
	#define SSD_CACHING_SPACE_BY_PAGES 64000//MAX:(6156008/SSD_PAGE2SECTOR)
	
	#define SSD_N_ELEMENTS 1 //預設為8//考慮移除:credit太大 //SSD Channels


	/*ipc*/
	#define KEY_MSQ_DISKSIM_1 0x1000
	#define KEY_MSQ_DISKSIM_2 0x2000
	#define MSG_TYPE_DISKSIM_1 100
	#define MSG_TYPE_DISKSIM_1_SERVED 101
	#define MSG_TYPE_DISKSIM_2 200
	#define MSG_TYPE_DISKSIM_2_SERVED 201
	#define MSG_REQUEST_CONTROL_FLAG_FINISH 999

	/*structure*/

	/*cache*/
	#define PAGE_FLAG_FREE 0
	#define PAGE_FLAG_CLEAN 1
	#define PAGE_FLAG_DIRTY -1
	#define CACHE_FULL 1
	#define CACHE_NOT_FULL 0

	/*prize caching*/
	#define MIN_PRIZE 0.0
	#define ALPHA 0.5
	
	/*credit*/
	#define INIT_CREDIT (TIME_PERIOD*SSD_N_ELEMENTS)

	/*Others*/
	//參考ASCII Escape Code
	#define PAUSE //getchar();
	#define COLOR_RB "\x1b[;31;1m"
	#define COLOR_GB "\x1b[;32;1m"
	#define COLOR_YB "\x1b[;33;1m"
	#define COLOR_BB "\x1b[;34;1m"
	#define COLOR_N "\x1b[0;m"

#endif
