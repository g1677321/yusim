#include "yusim.h"

pid_t SSDsimProc, HDDsimProc;
FILE *trace;
char *par[5];
double scheduleTime = 0;

unsigned long replenishmentCnt = 0;

/*DISKSIM INITIALIZATION*/
/**
 * [Disksim的初始化，利用兩個Process各自執行Disksim，作為SSDsim和HDDsim，
 *  接續MESSAGE QUEUE INITIALIZATION]
 */
void initDisksim() {
    pid_t procid;
    procid = fork();
    if (procid == 0) {
        SSDsimProc = getpid();
        //printf("SSDsimProc ID: %d\n", SSDsimProc);
        exec_SSDsim("SSDsim", par[1], par[2]);
        exit(0);
    }
    else if (procid < 0) {
        PrintError(-1, "SSDsim process fork() error");
        exit(1);
    }

    procid = fork();
    if (procid == 0) {
        HDDsimProc = getpid();
        //printf("HDDsimProc ID: %d\n", HDDsimProc);
        exec_HDDsim("HDDsim", par[3], par[4]);
        exit(0);
    }
    else if (procid < 0) {
        PrintError(-1, "HDDsim process fork() error");
        exit(1);
    }

    initMSQ();
}

/*DISKSIM SHUTDOWN*/
/**
 * [Disksim的關閉，傳送Control message 告知其Process進行Shutdown，並等待回傳結果message]
 */
void rmDisksim() {
    REQ *ctrl, *ctrl_rtn;
    ctrl = calloc(1, sizeof(REQ));
    ctrl_rtn = calloc(1, sizeof(REQ));
    ctrl->reqFlag = MSG_REQUEST_CONTROL_FLAG_FINISH; //非正常flag即可(ipc.c)
    
    sendFinishControl(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1);

    if(recvRequestByMSQ(KEY_MSQ_DISKSIM_1, ctrl_rtn, MSG_TYPE_DISKSIM_1_SERVED) == -1)
            PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
    printf(COLOR_YB"SSDsim response time = %lf\n"COLOR_N, ctrl_rtn->responseTime);

    sendFinishControl(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2);

    if(recvRequestByMSQ(KEY_MSQ_DISKSIM_2, ctrl_rtn, MSG_TYPE_DISKSIM_2_SERVED) == -1)
            PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
    printf(COLOR_YB"HDDsim response time = %lf\n"COLOR_N, ctrl_rtn->responseTime); 
    rmMSQ();
}

/*MESSAGE QUEUE INITIALIZATION*/
/**
 * [Message queue初始化，使用系統定義的Key值、Type和IPC function]
 */
void initMSQ() {
    if(createMessageQueue(KEY_MSQ_DISKSIM_1, IPC_CREAT) == -1)
        PrintError(-1, " MSQ create error in createMessageQueue():");
    if(createMessageQueue(KEY_MSQ_DISKSIM_2, IPC_CREAT) == -1)
        PrintError(-1, " MSQ create error in createMessageQueue():");
}

/*MESSAGE QUEUE REMOVE*/
/**
 * [Message queue刪除，使用系統定義的Key值和IPC function]
 */
void rmMSQ() {
    struct msqid_ds ds;
    if(removeMessageQueue(KEY_MSQ_DISKSIM_1, &ds) == -1)
        PrintError(KEY_MSQ_DISKSIM_1, "Not remove message queue:(key)");
    if(removeMessageQueue(KEY_MSQ_DISKSIM_2, &ds) == -1)
        PrintError(KEY_MSQ_DISKSIM_2, "Not remove message queue:(key)");
}

/*I/O SCHEDULING*/
/**
 * [Credit-based Scheduler，並且推算系統時間，決定Request delivery(Trace->User queue)]
 * @param {double} next_timout [The arrival time of the next request waiting for queueing]
 */
void scheduling(double next_timeout) {
    double response;
    int candidate;
    
    while (1) {
        //Credit-based Scheduler
        candidate = creditScheduler(userq);
        //User Queue無任何Requests，則Stop scheduling
        if (candidate == -1) {
            //printf("[YUSIM]No any request! Stop scheduling and delivery more request into 'userq'(%lf)\n", scheduleTime);
            break;
        }

        /*CACHING ALGORITHM*/
        //回傳可得response time
        response = prizeCaching(&userq[candidate].tail->r); //PRIZE.C
        //推算執行時間，為模擬系統時間
        //若目前的時間小於下一個request抵達的時間，則將系統時間往後推至下一個request完成的時間
        if (scheduleTime < userq[candidate].tail->r.arrivalTime)
            scheduleTime = response + userq[candidate].tail->r.arrivalTime;
        else //否則，只須累加下一個request的response time即可
            scheduleTime += response;
        //printf("[YUSIM]Blkno=%lu, Response time=%lf, scheduleTime=%lf\n", userq[candidate].tail->r.blkno, response, scheduleTime);
        evictQUE(candidate, userq[candidate].tail);
        
        /*CREDIT CHARGING*/
        //目前做法:假設一個Block Request做完後才扣Credit
        creditCharge(candidate, response);
        //printCredit();
        //creditCompensate();

        //根據TIME_PERIOD，週期性進行credit的補充
        if (floor(scheduleTime / TIME_PERIOD) - replenishmentCnt > 0) {
            replenishmentCnt = floor(scheduleTime / TIME_PERIOD);
            //printf("[YUSIM]creditReplenish()[%lu](%lf)\n", replenishmentCnt, scheduleTime);
            
            int i;
            for(i = 0; i < NUM_OF_USER; i++) {
                /*CREDIT REPLENISHMENT*/
                creditReplenish(i);
            }
            //printCredit();
        }

        //推算系統執行時間，是否有requests要進入User queue
        if (next_timeout != -1 && next_timeout <= scheduleTime) {
            //printf("[YUSIM]Some requests coming! Stop scheduling and delivery more request into 'userq'(%lf)\n", scheduleTime);
            break;
        }
    }
}

int main(int argc, char *argv[]) {
	if (argc != 6) {
    	fprintf(stderr, "usage: %s <Trace file> <param file for SSD> <output file for SSD> <param file for HDD> <output file for HDD>\n", argv[0]);
    	exit(1);
    }
    par[0] = argv[1];
    par[1] = argv[2];
    par[2] = argv[3];
    par[3] = argv[4];
    par[4] = argv[5];

    initDisksim();

    int i;
    for(i = 0; i < NUM_OF_USER; i++) {
        creditInit(i);
    }
    
    //printCredit();

    printf("[YUSIM]creditInit() finish!\n");

    printf("[YUSIM]Enter to continute ...\n");
    
    getchar();
    
    REQ *tmp, *rtn;
    tmp = calloc(1, sizeof(REQ));
    rtn = calloc(1, sizeof(REQ));

    trace = fopen(par[0], "r");
    if (!trace)
        PrintError(-1, "[YUSIM]Input file open error");

    while(!feof(trace)) {
        fscanf(trace, "%lf%u%lu%u%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->blkno, &tmp->reqSize, &tmp->reqFlag, &tmp->userno);
        //PrintREQ(tmp, "Trace");
        //getchar();

        //判斷時間間隔，用於策略的動態調整
        if (tmp->arrivalTime <= scheduleTime) {
            /*USER IDENTIFICATION*/
            if(insertQUE(tmp, tmp->userno-1) == -1)
                PrintError(-1, "[YUSIM]Error user or user queue! insertQUE():");
        }
        else {
            //printf(COLOR_RB"[YUSIM]Time to %lf\n"COLOR_N, scheduleTime);
            //printQUE();
            
            scheduling(tmp->arrivalTime);
            //metaTablePrint();

            PAUSE

            /*USER IDENTIFICATION*/
            if(insertQUE(tmp, tmp->userno-1) == -1)
                PrintError(-1, "[YUSIM]Error user or user queue!\n");
            /*NEXT TIME PERIOD*/
            //The N-th time period = (int)(tmp->arrivalTime/TIME_PERIOD + 1);N=1~n
            
        }
             
    }
    //printf(COLOR_RB"[YUSIM]Time to %lf\n"COLOR_N, scheduleTime);
    //printQUE();
    /*THE LAST TIME PERIOD*/
    
    scheduling(-1);
    //metaTablePrint();

    PAUSE

    rmDisksim();

    printf(COLOR_YB"[YUSIM]Receive requests:%lu\n"COLOR_N, getTotalReqs());
    //PRIZE.C
    pcStatistic();

	// Waiting for SSDsim and HDDsim process
    wait(NULL);
    wait(NULL);
    //OR
    //printf("Main Process waits for: %d\n", wait(NULL));
    //printf("Main Process waits for: %d\n", wait(NULL));

	exit(0);
}
