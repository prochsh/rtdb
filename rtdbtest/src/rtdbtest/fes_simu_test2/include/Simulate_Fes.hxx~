#ifndef _SIMULATE_FES_H_
#define _SIMULATE_FES_H_

#include <pthread.h>
#include <stdio.h>

int gen_random_subseq(int*subseq,int size,int range);
typedef struct
{
    long link_id;
    int link_no;
    int valid;
    short st_num;
    int *stations;
} SimFes_link;

typedef struct
{
    long station_id;		//厂站ID
    int station_no;		//厂站物理号
    int valid;
    int link_no;			//厂站所属链路的物理号
    int max_upana_no;
    int upana_num;
    int *upanalogs;
    int max_uppoi_no;
    int uppoi_num;
    int *uppoints;
} SimFes_station;

typedef struct
{
    long multi_analog_id;
    int multi_analog_no;
    long psid;
    float value;
    int status;
    float sample_value;
    int flag;
} SimFes_multi_analog;

typedef struct
{
    long analog_id;
    int analog_no;
    int station_no;
    int index_no;
    long psid;
    float max_raw_value;
    float min_raw_value;
    float raw_value;
    int status;
    SimFes_multi_analog *multi_analog_ptr;
    int flag;
    //int valid;
} SimFes_analog;

typedef struct
{
    long multi_point_id;
    int multi_point_no;
    long psid;
    float value;
    int status;
    float sample_value;
    int flag;
} SimFes_multi_point;

typedef struct
{
    long point_id;
    int point_no;
    int station_no;
    int index_no;
    long psid;
    float value;
    int status;
    SimFes_multi_point *multi_point_ptr;
    int flag;
    //int valid;
} SimFes_point;

typedef struct
{
    short table_no;
    int valid;
    int max_upana_num;
    int upana_num;
    int *upanalogs;
    int error;
    FILE *logfile;
} SimFes_sca_ana_dev_tab;

typedef struct
{
    short table_no;
    int valid;
    int max_uppoi_num;
    int uppoi_num;
    int *uppoints;
    FILE *logfile;
} SimFes_sca_poi_dev_tab;

#endif
