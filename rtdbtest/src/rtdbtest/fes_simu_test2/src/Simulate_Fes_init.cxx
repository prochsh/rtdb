#include "Simulate_Fes.hxx"
#include <system.h>
#include <db_struct_fes.h>
#include <lib_cdb.hxx>
#include <rtdb_api.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIMFES_REALTIME_NO	AC_REALTIME_NO
#define SIMFES_FES_NO	AP_FES
#define SIMFES_FES_LINKS_NO	1200
#define SIMFES_FES_STATIONS_NO 1201
#define SIMFES_FES_RECV_ANALOG_NO	1202
#define SIMFES_FES_RECV_POINT_NO	1203
#define SIMFES_FES_MULTI_ANALOG_NO	1218
#define SIMFES_FES_MULTI_POINT_NO	1219
#define VALID 1
#define INVALID	-1
#define MAXLINE	128
#define TABLE_NO_MASK	0xffff000000000000
#define MAX_FILE_NAME_LEN   64

extern float SimFes_ana_max_raw_value;
extern float SimFes_ana_min_raw_value;
extern int SimFes_ana_chg_interval;
extern int SimFes_poi_chg_interval;
extern int SimFes_ana_num_per_packet;
extern int SimFes_poi_num_per_packet;
extern int SimFes_max_ana_num_per_dev_tab;
extern int SimFes_max_poi_num_per_dev_tab;

extern SimFes_link *SimFes_links;
extern int SimFes_link_num;
extern int SimFes_max_link_num;

extern SimFes_station *SimFes_stations;
extern int SimFes_station_num;
extern int SimFes_max_station_num;

extern SimFes_analog *SimFes_upanalogs;
extern int SimFes_upanalog_num;
extern int SimFes_max_upanalog_num;
extern int SimFes_uniq_upanalog_num;
extern int SimFes_valid_upanalog_num;

extern SimFes_point *SimFes_uppoints;
extern int SimFes_uppoint_num;
extern int SimFes_max_uppoint_num;
extern int SimFes_uniq_uppoint_num;
extern int SimFes_valid_uppoint_num;

extern int SimFes_multi_upanalog_num;
extern int SimFes_max_multi_upanalog_num;
extern SimFes_multi_analog *SimFes_multi_upanalogs;

extern int SimFes_multi_uppoint_num;
extern int SimFes_max_multi_uppoint_num;
extern SimFes_multi_point *SimFes_multi_uppoints;

extern int SimFes_sca_ana_dev_tab_num;
extern SimFes_sca_ana_dev_tab *SimFes_sca_ana_dev_tabs;

extern int SimFes_sca_poi_dev_tab_num;
extern SimFes_sca_poi_dev_tab *SimFes_sca_poi_dev_tabs;

int cmp_ana_tab(const void *a,const void *b);
int cmp_poi_tab(const void *a,const void *b);
int cmp_mul_ana(const void *a,const void *b);
int cmp_mul_poi(const void *a,const void *b);
int cmp_ana(const void *a,const void *b);
int cmp_poi(const void *a,const void *b);
void get_time(char *time_str);
int analog_is_exist(long psid);



int gen_random_subseq(int*subseq,int size,int range){
	int i;
	srand((unsigned int)time(NULL));
    for(i=0;i<size;i++){
		subseq[i]=range*(rand()/(RAND_MAX+1.0));
	}
}
	



//功能:	从配置文件中初始化参数
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_config(void)
{
    char *d5000_home;
    if((d5000_home=getenv("D5000_HOME"))==NULL)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]getenv $D5000_HOME failed!\n",__FILE__,__LINE__,__FUNCTION__);
        return -1;
    }
    char confFile[128];
    bzero(confFile,sizeof(confFile));
    sprintf(confFile,"%s/conf/fes_simu.ini",d5000_home);

    FILE *fp;
    if((fp=fopen(confFile,"r"))==NULL)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]open config file failed\n",__FILE__,__LINE__,__FUNCTION__);
        return -1;
    }

    char buf[MAXLINE];
    while(fgets(buf,MAXLINE,fp)!=NULL)
    {
		printf("[DEBUG] buf=[%s]\n",buf);
        char *key;
        char *value;

        if(buf[0]=='#')
            continue;
        key=strtok(buf,"=");
        value=strtok(NULL,"\n");
#ifdef SIMFES_DEBUG
        printf("%s:%d:%s	[DEBUG]key='%s',value='%s'\n",__FILE__,__LINE__,__FUNCTION__,key,value);
#endif
        if(!strcmp(key,"ana_max_raw_value"))
        {
            SimFes_ana_max_raw_value=strtof(value,NULL);
        }
        else if(!strcmp(key,"ana_min_raw_value"))
        {
            SimFes_ana_min_raw_value=strtof(value,NULL);
        }
        else if(!strcmp(key,"ana_chg_interval"))
        {
            SimFes_ana_chg_interval=atoi(value);
        }
        else if(!strcmp(key,"poi_chg_interval"))
        {
            SimFes_poi_chg_interval=atoi(value);
        }
        else if(!strcmp(key,"ana_num_per_packet"))
        {
            SimFes_ana_num_per_packet=atoi(value);
        }
        else if(!strcmp(key,"poi_num_per_packet"))
        {
            SimFes_poi_num_per_packet=atoi(value);
        }
        else if(!strcmp(key,"max_ana_num_per_dev_tab"))
        {
            SimFes_max_ana_num_per_dev_tab=atoi(value);
        }
        else if(!strcmp(key,"max_poi_num_per_dev_tab"))
        {
            SimFes_max_poi_num_per_dev_tab=atoi(value);
        }
        else if(!strcmp(key,"sca_ana_dev_tab_num"))
        {
            SimFes_sca_ana_dev_tab_num=atoi(value);
            if((SimFes_sca_ana_dev_tabs=(SimFes_sca_ana_dev_tab *)malloc(SimFes_sca_ana_dev_tab_num*sizeof(SimFes_sca_ana_dev_tab)))==NULL)
            {
                fprintf(stdin,"%s:%d:%s		[ERROR]malloc failed",__FILE__,__LINE__,__FUNCTION__);
                return -1;
            }
            for(int i=0; i<SimFes_sca_ana_dev_tab_num; i++)
            {
                SimFes_sca_ana_dev_tabs[i].valid=INVALID;
                SimFes_sca_ana_dev_tabs[i].upanalogs=NULL;
                SimFes_sca_ana_dev_tabs[i].upana_num=0;
                SimFes_sca_ana_dev_tabs[i].logfile=NULL;
            }
        }
        else if(!strcmp(key,"sca_ana_dev_tab"))
        {
            int tab_num;
            char *tab_no;

            tab_num=0;
            tab_no=strtok(value,"\t\n");
            while(tab_no)
            {
                tab_num++;
                if(tab_num>SimFes_sca_ana_dev_tab_num)
                {
                    fprintf(stdin,"%s:%d:%s		[WARN]tab_num %d > sca_ana_dev_tab %d",__FILE__,__LINE__,__FUNCTION__,tab_num,SimFes_sca_ana_dev_tab_num);
                    break;
                }
                SimFes_sca_ana_dev_tabs[tab_num-1].table_no=atoi(tab_no);
				printf("tab_num=%d tab_no=%d\n",tab_num,atoi(tab_no));
                int *upanalogs;
                if((upanalogs=(int *)malloc(SimFes_max_ana_num_per_dev_tab*sizeof(int)))==NULL)
                {
                    fprintf(stdin,"%s:%d:%s		[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
                    return -1;
                }
                SimFes_sca_ana_dev_tabs[tab_num-1].upanalogs=upanalogs;
                FILE *fp;
                char logfile[MAX_FILE_NAME_LEN];
                sprintf(logfile,"%s/var/log/fes/fes_simu/%d_analog",d5000_home,SimFes_sca_ana_dev_tabs[tab_num-1].table_no);
                if((fp=fopen(logfile,"w"))==NULL)
                {
                    fprintf(stdin,"%s:%d:%s		[ERROR]open %s failed\n",__FILE__,__LINE__,__FUNCTION__,logfile);
                    return -1;
                }
                SimFes_sca_ana_dev_tabs[tab_num-1].logfile=fp;
                SimFes_sca_ana_dev_tabs[tab_num-1].valid=VALID;
                tab_no=strtok(NULL,"\t\n");
            }
			printf("%s:%d	[DEBUG]succeed\n",__FILE__,__LINE__);
            if(tab_num!=SimFes_sca_ana_dev_tab_num)
            {
                fprintf(stdin,"%s:%d:%s		[WARN]tab_num %d != sca_ana_dev_tab_num %d\n",__FILE__,__LINE__,__FUNCTION__,tab_num,SimFes_sca_ana_dev_tab_num);
            }
        }
        else if(!strcmp(key,"sca_poi_dev_tab_num"))
        {
            SimFes_sca_poi_dev_tab_num=atoi(value);
            if((SimFes_sca_poi_dev_tabs=(SimFes_sca_poi_dev_tab *)malloc(SimFes_sca_poi_dev_tab_num*sizeof(SimFes_sca_poi_dev_tab)))==NULL)
            {
                fprintf(stdin,"%s:%d:%s		[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
                return -1;
            }
            for(int i=0; i<SimFes_sca_poi_dev_tab_num; i++)
            {
                SimFes_sca_poi_dev_tabs[i].valid=INVALID;
                SimFes_sca_poi_dev_tabs[i].uppoints=NULL;
                SimFes_sca_poi_dev_tabs[i].uppoi_num=0;
                SimFes_sca_poi_dev_tabs[i].logfile=NULL;
            }
        }
        else if(!strcmp(key,"sca_poi_dev_tab"))
        {
            int tab_num;
            char *tab_no;

            tab_num=0;
            tab_no=strtok(value,"\t\n");
            while(tab_no)
            {
                tab_num++;
                if(tab_num>SimFes_sca_poi_dev_tab_num)
                {
                    fprintf(stdin,"%s:%d:%s		[WARN]tab_num %d > sca_poi_dev_tab %d\n",__FILE__,__LINE__,__FUNCTION__,tab_num,SimFes_sca_poi_dev_tab_num);
                    break;
                }
                SimFes_sca_poi_dev_tabs[tab_num-1].table_no=atoi(tab_no);
                int *uppoints;
                if((uppoints=(int *)malloc(SimFes_max_poi_num_per_dev_tab*sizeof(int)))==NULL)
                {
                    fprintf(stdin,"%s:%d:%s		[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
                    return -1;
                }
                SimFes_sca_poi_dev_tabs[tab_num-1].uppoints=uppoints;
                SimFes_sca_poi_dev_tabs[tab_num-1].valid=VALID;
                tab_no=strtok(NULL,"\t");
            }
            if(tab_num!=SimFes_sca_poi_dev_tab_num)
            {
                fprintf(stdin,"%s:%d:%s		[WARN]tab_num %d != sca_poi_dev_tab %d\n",__FILE__,__LINE__,__FUNCTION__,tab_num,SimFes_sca_poi_dev_tab_num);
            }
        }
        else
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]init para from config failed!\n",__FILE__,__LINE__,__FUNCTION__);
            return -1;
        }
    }
    if(ferror(fp))
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]read config error\n",__FILE__,__LINE__,__FUNCTION__);
        return -1;
    }
#ifdef SIMFES_DEBUG
    printf("%s:%d:%s		[DEBUG]ana_max_raw_value=%f\n",__FILE__,__LINE__,__FUNCTION__,SimFes_ana_max_raw_value);
    printf("%s:%d:%s    [DEBUG]ana_min_raw_value=%f\n",__FILE__,__LINE__,__FUNCTION__,SimFes_ana_min_raw_value);
    printf("%s:%d:%s    [DEBUG]ana_chg_interval=%d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_ana_chg_interval);
    printf("%s:%d:%s    [DEBUG]poi_chg_interval=%d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_poi_chg_interval);
    printf("%s:%d:%s    [DEBUG]ana_num_per_packet=%d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_ana_num_per_packet);
    printf("%s:%d:%s    [DEBUG]poi_num_per_packet=%d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_poi_num_per_packet);
    printf("%s:%d:%s    [DEBUG]max_ana_num_per_dev_tab=%d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_max_ana_num_per_dev_tab);
    printf("%s:%d:%s    [DEBUG]max_poi_num_per_dev_tab=%d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_max_poi_num_per_dev_tab);
    printf("%s:%d:%s    [DEBUG]sca_ana_dev_tab_num=%d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_sca_ana_dev_tab_num);
    printf("%s:%d:%s    [DEBUG]sca_ana_dev_tab=",__FILE__,__LINE__,__FUNCTION__);
    for(int i=0; i<SimFes_sca_ana_dev_tab_num; i++)
    {
        if(SimFes_sca_ana_dev_tabs[i].valid==VALID)
        {
            printf("%d\t",SimFes_sca_ana_dev_tabs[i].table_no);
        }
    }
    printf("\n");
    printf("%s:%d:%s    [DEBUG]sca_poi_dev_tab_num=%d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_sca_poi_dev_tab_num);
    printf("%s:%d:%s    [DEBUG]sca_poi_dev_tab=",__FILE__,__LINE__,__FUNCTION__);
    for(int i=0; i<SimFes_sca_poi_dev_tab_num; i++)
    {
        if(SimFes_sca_poi_dev_tabs[i].valid==VALID)
        {
            printf("%d\t",SimFes_sca_poi_dev_tabs[i].table_no);
        }
    }
    printf("\n");
    sleep(5);
#endif
    return 0;
}

//功能:	根据实时库初始化链路结构
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_init_link(void)
{
    //打开表Fes/link_conf
    TB_DESCR tb_des;
    int ret_val;

    if((ret_val=OpenTableByID(NULL,SIMFES_REALTIME_NO,SIMFES_FES_NO,SIMFES_FES_LINKS_NO,&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when OpenTableByID %d,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_LINKS_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]OpenTableById %d succeed\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_LINKS_NO);

    //读表参数
    TABLE_PARA_STRU table_para;
    if((ret_val=GetTablePara(&tb_des,&table_para))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when  GetTablePara %d,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_LINKS_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]GetTablePara %d succeed\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_LINKS_NO);
    SimFes_max_link_num=table_para.max_valid_rec;
    printf("%s:%d:%s	[DEBUG]max_valid_rec=%d in table %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_max_link_num,SIMFES_FES_LINKS_NO);

    //分配链路结构空间
    if((SimFes_links=(SimFes_link *)malloc(SimFes_max_link_num*sizeof(SimFes_link)))==NULL)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
        return -1;
    }
    printf("%s:%d:%s	malloc succeed\n",__FILE__,__LINE__,__FUNCTION__);

    //初始化链路参数
    link_conf_fes *db_links_info;
    if((ret_val=GetTableRecs(&tb_des,0,(char **)&db_links_info))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]read table %d records,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_LINKS_NO,ret_val);
        return -1;
    }
    SimFes_link_num= ret_val/sizeof(link_conf_fes);
    printf("%s:%d:%s	[DEBUG]there are %d links in table %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_link_num,SIMFES_FES_LINKS_NO);


    for(int i=0; i<SimFes_link_num; i++)
    {
        int phy_no;
        phy_no=((KEY_STRUCT *)&(db_links_info[i].linkid))->record_no - 1;

        SimFes_links[phy_no].link_id=db_links_info[i].linkid;
        SimFes_links[phy_no].link_no=phy_no;

        SimFes_links[phy_no].st_num=db_links_info[i].stationnum;
        int *stations;
        if((stations=(int *)malloc(db_links_info[i].stationnum*sizeof(int)))==NULL)
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
            return -1;
        }
        for(int j=0; j<db_links_info[i].stationnum; j++)
        {
            stations[j]=INVALID;
        }
        SimFes_links[phy_no].stations=stations;

        SimFes_links[phy_no].valid=VALID;

#ifdef SIMFES_DEBUG
        printf("%s:%d:%s    [DEBUG]phyno:%d,link_id:%ld,link_no:%d,st_num:%d\n",__FILE__,__LINE__,__FUNCTION__,phy_no,SimFes_links[phy_no].link_id,SimFes_links[phy_no].link_no,SimFes_links[phy_no].st_num);
#endif
    }
    if((ret_val=CloseTable(&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]close table %d failed,ret_val=%d",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_LINKS_NO,ret_val);
        return -1;
    }

    return 0;
}


//功能:	根据实时库初始化厂站结构
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_init_station(void)
{
    short link_counter[SimFes_max_link_num];
    bzero(link_counter,SimFes_max_link_num*sizeof(short));

    //打开表Fes/st_conf
    TB_DESCR tb_des;
    int ret_val;

    if((ret_val=OpenTableByID(NULL,SIMFES_REALTIME_NO,SIMFES_FES_NO,SIMFES_FES_STATIONS_NO,&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when OpenTableByID %d,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_STATIONS_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]OpenTableById %d succeed\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_STATIONS_NO);

    //读表参数
    TABLE_PARA_STRU table_para;
    if((ret_val=GetTablePara(&tb_des,&table_para))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when  GetTablePara %d,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_STATIONS_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]GetTablePara %d succeed\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_STATIONS_NO);
    SimFes_max_station_num=table_para.max_valid_rec;
    printf("%s:%d:%s	[DEBUG]max_valid_rec=%d in table %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_max_station_num,SIMFES_FES_STATIONS_NO);

    //分配厂站结构空间
    if((SimFes_stations=(SimFes_station *)malloc(SimFes_max_station_num*sizeof(SimFes_station)))==NULL)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]malloc succeed\n",__FILE__,__LINE__,__FUNCTION__);

    //初始化厂站参数
    st_conf_fes *db_stations_info;
    if((ret_val=GetTableRecs(&tb_des,0,(char **)&db_stations_info))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]read table %d records,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_STATIONS_NO,ret_val);
        return -1;
    }
    SimFes_station_num= ret_val/sizeof(st_conf_fes);
    printf("%s:%d:%s	[DEBUG]there are %d stations in table %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_station_num,SIMFES_FES_STATIONS_NO);


    for(int i=0; i<SimFes_station_num; i++)
    {
        int phy_no;
        phy_no=((KEY_STRUCT *)&(db_stations_info[i].st_id))->record_no - 1;

        //查找厂站所属链路
        int link_no;
        link_no=((KEY_STRUCT *)&(db_stations_info[i].linkid))->record_no - 1;
        if(SimFes_links[link_no].st_num <= 0) // 如果通道的厂站个数为0，那么不初始化该厂站
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]st_num %d <= 0 for link %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_links[link_no].st_num,link_no);
            continue;
        }
        if(link_counter[link_no] >= SimFes_links[link_no].st_num) // 如果厂站计数超过了通道的厂站个数,那么不初始化该厂站
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]count %d > st_num %d for link %d\n",__FILE__,__LINE__,__FUNCTION__,link_counter[link_no],SimFes_links[link_no].st_num,link_no);
            continue;
        }

        SimFes_stations[phy_no].station_id=db_stations_info[i].st_id;
        SimFes_stations[phy_no].station_no=phy_no;

        SimFes_stations[phy_no].link_no=link_no;
        SimFes_links[link_no].stations[link_counter[link_no]]=phy_no;

        SimFes_stations[phy_no].max_upana_no=db_stations_info[i].max_upana_no;
        int *upanalogs;
        if((upanalogs=(int *)malloc(db_stations_info[i].max_upana_no*sizeof(int)))==NULL)
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
            return -1;
        }
        for(int j=0; j<db_stations_info[i].max_upana_no; j++)
        {
            upanalogs[j]=INVALID;
        }
        SimFes_stations[phy_no].upanalogs=upanalogs;
        SimFes_stations[phy_no].upana_num=0;

        SimFes_stations[phy_no].max_uppoi_no=db_stations_info[i].max_uppoi_no;
        int *uppoints;
        if((uppoints=(int *)malloc(db_stations_info[i].max_uppoi_no*sizeof(int)))==NULL)
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
            return -1;
        }
        for(int j=0; j<db_stations_info[i].max_uppoi_no; j++)
        {
            uppoints[j]=INVALID;
        }
        SimFes_stations[phy_no].uppoints=uppoints;
        SimFes_stations[phy_no].uppoi_num=0;
        SimFes_stations[phy_no].valid=VALID;
#ifdef SIMFES_DEBUG
        printf("%s:%d:%s	[DEBUG]phy_no:%d,link_no:%d,station_id:%ld,station_no:%d,max_upana_no:%d,.max_uppoi_no:%d\n",__FILE__,__LINE__,__FUNCTION__,phy_no,SimFes_stations[phy_no].link_no,SimFes_stations[phy_no].station_id,SimFes_stations[phy_no].station_no,SimFes_stations[phy_no].max_upana_no, SimFes_stations[phy_no].max_uppoi_no);
#endif
        link_counter[link_no]++;
    }
    if((ret_val=CloseTable(&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]close table %d failed,ret_val=%d",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_STATIONS_NO,ret_val);
        return -1;
    }

    return 0;
}

//功能:	根据实时库初始化多源遥测结构
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_init_multi_upana(void)
{
    multi_analog_fes *db_multi_upana_info;
    TB_DESCR tb_des;
    int ret_val;

    //打开表Fes/multi_analog
    if((ret_val=OpenTableByID(NULL,SIMFES_REALTIME_NO,SIMFES_FES_NO,SIMFES_FES_MULTI_ANALOG_NO,&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when OpenTableByID %d,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_ANALOG_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]OpenTableById %d succeed\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_ANALOG_NO);

    //读表参数
    TABLE_PARA_STRU table_para;
    if((ret_val=GetTablePara(&tb_des,&table_para))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when  GetTablePara %d,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_ANALOG_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]GetTablePara %d succeed\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_ANALOG_NO);
    SimFes_max_multi_upanalog_num=table_para.max_valid_rec;
    printf("%s:%d:%s	[DEBUG]max_valid_rec=%d in talbe %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_max_multi_upanalog_num,SIMFES_FES_MULTI_ANALOG_NO);
    sleep(10);

    //分配上行遥测结构空间
    if((SimFes_multi_upanalogs=(SimFes_multi_analog *)malloc(SimFes_max_multi_upanalog_num*sizeof(SimFes_multi_analog)))==NULL)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]malloc succeed\n",__FILE__,__LINE__,__FUNCTION__);

    //初始化上行遥测结构
    if((ret_val=GetTableRecs(&tb_des,0,(char **)&db_multi_upana_info))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]read table %d records failed,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_ANALOG_NO,ret_val);
        return -1;
    }
    SimFes_multi_upanalog_num= ret_val/sizeof(multi_analog_fes);
    printf("%s:%d:%s	[DEBUG]there are %d multi upanalogs in table %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_multi_upanalog_num,SIMFES_FES_MULTI_ANALOG_NO);
    sleep(5);

    int i=0;
    while(i<SimFes_multi_upanalog_num)
    {
        int phy_no;
        phy_no=((KEY_STRUCT *)&(db_multi_upana_info[i].id))->record_no - 1;

        SimFes_multi_upanalogs[i].multi_analog_id=db_multi_upana_info[i].id;
        SimFes_multi_upanalogs[i].multi_analog_no=phy_no;
        SimFes_multi_upanalogs[i].psid=db_multi_upana_info[i].psid;
		i++;
    }
    if((ret_val=CloseTable(&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]close table %d failed,ret_val=%d",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_ANALOG_NO,ret_val);
        return -1;
    }

    return 0;
}

//功能:	根据实时库初始化多源遥信结构
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_init_multi_uppoi(void)
{
    multi_point_fes *db_multi_uppoi_info;
    TB_DESCR tb_des;
    int ret_val;

    //打开表Fes/multi_analog
    if((ret_val=OpenTableByID(NULL,SIMFES_REALTIME_NO,SIMFES_FES_NO,SIMFES_FES_MULTI_POINT_NO,&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when OpenTableByID %d,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_POINT_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]OpenTableById %d succeed\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_POINT_NO);

    //读表参数
    TABLE_PARA_STRU table_para;
    if((ret_val=GetTablePara(&tb_des,&table_para))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when  GetTablePara %d,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_POINT_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]GetTablePara %d succeed\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_POINT_NO);
    SimFes_max_multi_uppoint_num=table_para.max_valid_rec;
    printf("%s:%d:%s	[DEBUG]max_valid_rec=%d in talbe %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_max_multi_uppoint_num,SIMFES_FES_MULTI_POINT_NO);
    sleep(10);

    //分配上行遥测结构空间
    if((SimFes_multi_uppoints=(SimFes_multi_point *)malloc(SimFes_max_multi_uppoint_num*sizeof(SimFes_multi_point)))==NULL)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]malloc succeed\n",__FILE__,__LINE__,__FUNCTION__);

    //初始化上行遥测结构
    if((ret_val=GetTableRecs(&tb_des,0,(char **)&db_multi_uppoi_info))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]read table %d records failed,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_POINT_NO,ret_val);
        return -1;
    }
    SimFes_multi_uppoint_num= ret_val/sizeof(multi_point_fes);
    printf("%s:%d:%s	[DEBUG]there are %d multi uppoints in table %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_multi_uppoint_num,SIMFES_FES_MULTI_POINT_NO);
    sleep(10);

    int i=0;
    while(i<SimFes_multi_uppoint_num)
    {
        int phy_no;
        phy_no=((KEY_STRUCT *)&(db_multi_uppoi_info[i].id))->record_no - 1;

        SimFes_multi_uppoints[i].multi_point_id=db_multi_uppoi_info[i].id;
        SimFes_multi_uppoints[i].multi_point_no=phy_no;
        SimFes_multi_uppoints[i].psid=db_multi_uppoi_info[i].psid;
		i++;
    }
    if((ret_val=CloseTable(&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]close table %d failed,ret_val=%d",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_MULTI_POINT_NO,ret_val);
        return -1;
    }

    return 0;
}

//功能:	按psid字段排序多源遥测点
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_sort_multi_upana_by_psid(void)
{
    qsort(SimFes_multi_upanalogs,SimFes_multi_upanalog_num,sizeof(SimFes_multi_upanalogs[0]),cmp_mul_ana);
}

//功能:	按psid字段排序多源遥信点
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_sort_multi_uppoi_by_psid(void)
{
    qsort(SimFes_multi_uppoints,SimFes_multi_uppoint_num,sizeof(SimFes_multi_point),cmp_mul_poi);
}

//功能:	算子，按psid字段比较多源遥测点
//参数:	待比较元素
//返回值:	大于-'1';
//		小于-'-1'
int cmp_mul_ana(const void *a,const void *b)
{
	if((*(SimFes_multi_analog*)a).psid > (*(SimFes_multi_analog*)b).psid)
	{
		return 1;
	}else if((*(SimFes_multi_analog*)a).psid < (*(SimFes_multi_analog*)b).psid)
	{
		return -1;
	}else
	{
		return 0;
	}
    //return (*(SimFes_multi_analog*)a).psid - (*(SimFes_multi_analog*)b).psid;
}

//功能:	算子，按psid字段比较多源遥信点
//参数:	待比较元素
//返回值:	大于-'1';
//		小于-'-1'
int cmp_mul_poi(const void *a,const void *b)
{
    if((*(SimFes_multi_point*)a).psid > (*(SimFes_multi_point*)b).psid)
	{
		return 1;
	}else if((*(SimFes_multi_point*)a).psid < (*(SimFes_multi_point*)b).psid)
	{
		return -1;
	}else
	{
		return 0;
	}

	//return (*(SimFes_multi_point*)a).psid - (*(SimFes_multi_point*)b).psid;
}

//功能:	根据实时库初始化遥测结构
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_init_upana(void)
{
    char * d5000_home;
    char time_str[32];
    FILE * analog_fp_valid;
    FILE * analog_fp_unvalid;
    char analog_file_valid[MAX_FILE_NAME_LEN];
    char analog_file_unvalid[MAX_FILE_NAME_LEN];

    if((d5000_home=getenv("D5000_HOME"))==NULL)
    {
        fprintf(stdin,"%s:%d:%s		[ERROR]getenv $D5000_HOME failed\n",__FILE__,__LINE__,__FUNCTION__);
        return -1;
    }
    sprintf(analog_file_valid,"%s/var/log/fes/fes_simu/analog.valid",d5000_home);
    if((analog_fp_valid=fopen(analog_file_valid,"w"))==NULL)
    {
        fprintf(stdin,"%s:%d:%s		[ERROR]open %s failed\n",__FILE__,__LINE__,__FUNCTION__,analog_file_valid);
        return -1;
    }
    sprintf(analog_file_unvalid,"%s/var/log/fes/fes_simu/analog.unvalid",d5000_home);
    if((analog_fp_unvalid=fopen(analog_file_unvalid,"w"))==NULL)
    {
        fprintf(stdin,"%s:%d:%s		[ERROR]open %s failed\n",__FILE__,__LINE__,__FUNCTION__,analog_file_unvalid);
        return -1;
    }

    //打开表Fes/recv_analog
    recv_analog_fes *db_upana_info;
    TB_DESCR tb_des;
    int ret_val;

    if((ret_val=OpenTableByID(NULL,SIMFES_REALTIME_NO,SIMFES_FES_NO,SIMFES_FES_RECV_ANALOG_NO,&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when OpenTableByID %d,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_ANALOG_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]OpenTableById %d succeed\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_ANALOG_NO);

    //读表参数
    TABLE_PARA_STRU table_para;
    if((ret_val=GetTablePara(&tb_des,&table_para))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when  GetTablePara %d,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_ANALOG_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]GetTablePara %d succeed\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_ANALOG_NO);
    SimFes_max_upanalog_num=table_para.max_valid_rec;
    printf("%s:%d:%s	[DEBUG]max_valid_rec=%d in talbe %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_max_upanalog_num,SIMFES_FES_RECV_ANALOG_NO);
    sleep(5);

    //分配上行遥测结构空间
    if((SimFes_upanalogs=(SimFes_analog *)malloc(SimFes_max_upanalog_num*sizeof(SimFes_analog)))==NULL)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]malloc failed\n",__FILE__,__LINE__,__FUNCTION__);
        return -1;
    }
    printf("%s:%d:%s	malloc succeed\n",__FILE__,__LINE__,__FUNCTION__);

    /**
    	for(int i=0; i<SimFes_max_upanalog_num; i++)
       	 SimFes_upanalogs[i].valid=INVALID;
    */
    //初始化上行遥测结构
    if((ret_val=GetTableRecs(&tb_des,0,(char **)&db_upana_info))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]read table %d records failed,ret_val=%d\n",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_ANALOG_NO,ret_val);
        return -1;
    }
    SimFes_upanalog_num= ret_val/sizeof(recv_analog_fes);
    printf("%s:%d:%s	[DEBUG]there are %d upanalogs in table %d\n",__FILE__,__LINE__,__FUNCTION__,SimFes_upanalog_num,SIMFES_FES_RECV_ANALOG_NO);
    sleep(5);

    int i,j;
    i=0;
    j=0;
    while(i<SimFes_upanalog_num)
    {
        int phy_no;
        phy_no=((KEY_STRUCT *)&(db_upana_info[i].id))->record_no - 1;

        if((db_upana_info[i].stno <= 0) || (db_upana_info[i].stno > SimFes_max_station_num))
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]stno %d = 0 or st_no %d > max_st_num %d for upanalog %d\n",__FILE__,__LINE__,__FUNCTION__,db_upana_info[i].stno,db_upana_info[i].stno,SimFes_max_station_num,phy_no);
            get_time(time_str);
            fprintf(analog_fp_unvalid,"%s	[DEBUG]offset:%d,phy_no:%d,analog_id:%ld,psid:%ld [stno %d = 0 or st_no %d > max_st_num %d]\n",time_str,i,phy_no,db_upana_info[i].id,db_upana_info[i].psid,db_upana_info[i].stno,db_upana_info[i].stno,SimFes_max_station_num);
            i++;
            continue;
        }

        int station_no;
        station_no=db_upana_info[i].stno - 1;
        if(SimFes_stations[station_no].max_upana_no == 0)
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]st_no %d  max_upana_no 0 for upanalog %d\n",__FILE__,__LINE__,__FUNCTION__,station_no,phy_no);
            get_time(time_str);
            fprintf(analog_fp_unvalid,"%s	[DEBUG]offset:%d,phy_no:%d,analog_id:%ld,psid:%ld	[st_no %d  max_upana_no 0]\n",time_str,i,phy_no,db_upana_info[i].id,db_upana_info[i].psid,station_no);
            i++;
            continue;
        }
        int index_no;
        index_no = db_upana_info[i].index_no;
        if((index_no < 0) || (index_no > SimFes_stations[station_no].max_upana_no))
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]index_no %d > max_upana_no %d for upanalog %d\n",__FILE__,__LINE__,__FUNCTION__,index_no,SimFes_stations[station_no].max_upana_no,phy_no);
            get_time(time_str);
            fprintf(analog_fp_unvalid,"%s	[DEBUG]offset:%d,phy_no:%d,analog_id:%ld,psid:%ld	[index_no %d > max_upana_no %d]\n",time_str,i,phy_no,db_upana_info[i].id,db_upana_info[i].psid,index_no,SimFes_stations[station_no].max_upana_no);
            i++;
            continue;
        }
		if (analog_is_exist(db_upana_info[i].psid)==0){	
		    i++;
			continue;
		}

        SimFes_upanalogs[j].analog_id=db_upana_info[i].id;
        SimFes_upanalogs[j].analog_no=phy_no;
        SimFes_upanalogs[j].station_no=station_no;
        SimFes_upanalogs[j].index_no=index_no;
        SimFes_upanalogs[j].psid=db_upana_info[i].psid;
        SimFes_stations[station_no].upanalogs[index_no] = j;
        SimFes_stations[station_no].upana_num++;

        const float EPSINON=0.0000001;
        SimFes_upanalogs[j].max_raw_value=db_upana_info[i].rawmax;
        if(fabs(SimFes_upanalogs[j].max_raw_value) <= EPSINON)
        {
            SimFes_upanalogs[j].max_raw_value=SimFes_ana_max_raw_value;
        }
        SimFes_upanalogs[j].min_raw_value=db_upana_info[i].rawmin;
        if(SimFes_upanalogs[j].min_raw_value < -EPSINON)
        {
            SimFes_upanalogs[j].max_raw_value=SimFes_ana_min_raw_value;
        }
        SimFes_upanalogs[j].raw_value=0;
        SimFes_upanalogs[j].status=0;

        SimFes_multi_analog	tmp;
        tmp.psid=SimFes_upanalogs[j].psid;
        SimFes_upanalogs[j].multi_analog_ptr=(SimFes_multi_analog*)bsearch(&tmp,SimFes_multi_upanalogs,SimFes_multi_upanalog_num,sizeof(SimFes_multi_upanalogs[0]),cmp_mul_ana);

        //SimFes_upanalogs[j].valid=VALID;
#ifdef SIMFES_DEBUG
        printf("%s:%d:%s	[DEBUG]phy_no:%d,station_no:%d,analog_id:%ld,analog_no:%d,index_no:%d,psid:%ld,max_raw_value:%f,min_raw_value:%f\n",__FILE__,__LINE__,__FUNCTION__,phy_no,SimFes_upanalogs[j].station_no,SimFes_upanalogs[j].analog_id,SimFes_upanalogs[j].analog_no,SimFes_upanalogs[j].index_no,SimFes_upanalogs[j].psid,SimFes_upanalogs[j].max_raw_value,SimFes_upanalogs[j].min_raw_value);
#endif
        get_time(time_str);
        fprintf(analog_fp_valid,"%s	[DEBUG]offset:%d,phy_no:%d,station_no:%d,analog_id:%ld,analog_no:%d,index_no:%d,psid:%ld,max_raw_value:%f,min_raw_value:%f\n",time_str,i,phy_no,SimFes_upanalogs[j].station_no,SimFes_upanalogs[j].analog_id,SimFes_upanalogs[j].analog_no,SimFes_upanalogs[j].index_no,SimFes_upanalogs[j].psid,SimFes_upanalogs[j].max_raw_value,SimFes_upanalogs[j].min_raw_value);
    	i++;
        j++;
	}
    SimFes_valid_upanalog_num=j;

    if((ret_val=CloseTable(&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]close table %d failed,ret_val=%d",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_ANALOG_NO,ret_val);
        return -1;
    }
    fclose(analog_fp_valid);
    fclose(analog_fp_unvalid);

    return 0;
}

//功能:	根据实时库初始化遥信结构
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_init_uppoi(void)
{
    recv_point_fes *db_uppoi_info;

    //打开表Fes/recv_point
    TB_DESCR tb_des;
    int ret_val;

    if((ret_val=OpenTableByID(NULL,SIMFES_REALTIME_NO,SIMFES_FES_NO,SIMFES_FES_RECV_POINT_NO,&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when OpenTableByID %d,ret_val=%d",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_POINT_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]OpenTableById %d succeed",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_POINT_NO);

    //读表参数
    TABLE_PARA_STRU table_para;
    if((ret_val=GetTablePara(&tb_des,&table_para))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]error when  GetTablePara %d,ret_val=%d",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_POINT_NO,ret_val);
        return -1;
    }
    printf("%s:%d:%s	[DEBUG]GetTablePara %d succeed",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_POINT_NO);
    SimFes_max_uppoint_num=table_para.max_valid_rec;
    printf("%s:%d:%s	[DEBUG]max_valid_rec=%d in talbe %d",__FILE__,__LINE__,__FUNCTION__,SimFes_max_uppoint_num,SIMFES_FES_RECV_POINT_NO);

    //分配上行遥测结构空间
    if((SimFes_uppoints=(SimFes_point *)malloc(SimFes_max_uppoint_num*sizeof(SimFes_point)))==NULL)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]malloc failed",__FILE__,__LINE__,__FUNCTION__);
        return -1;
    }
    printf("%s:%d:%s	malloc succeed",__FILE__,__LINE__,__FUNCTION__);

    /**
    	for(int i=0; i<SimFes_max_uppoint_num; i++)
       	SimFes_uppoints[i].valid=INVALID;
    */
    //初始化上行遥测结构
    if((ret_val=GetTableRecs(&tb_des,0,(char **)&db_uppoi_info))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]read table %d records failed,ret_val=%d",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_POINT_NO,ret_val);
        return -1;
    }
    SimFes_uppoint_num= ret_val/sizeof(recv_point_fes);
    printf("%s:%d:%s	[DEBUG]there are %d uppoints in table %d",__FILE__,__LINE__,__FUNCTION__,SimFes_uppoint_num,SIMFES_FES_RECV_POINT_NO);

    int i,j;
    i=0;
    j=0;
    while(i<SimFes_uppoint_num)
    {
        int phy_no;
        phy_no=((KEY_STRUCT *)&(db_uppoi_info[i].id))->record_no - 1;

        if((db_uppoi_info[i].stno <= 0) || (db_uppoi_info[i].stno > SimFes_max_station_num))
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]st_no %d > max_st_num %d for uppoint %d\n",__FILE__,__LINE__,__FUNCTION__,db_uppoi_info[i].stno,SimFes_max_station_num,phy_no);
            i++;
            continue;
        }

        int station_no;
        station_no=db_uppoi_info[i].stno - 1;
        if(SimFes_stations[station_no].max_uppoi_no == 0)
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]st_no %d  max_uppoint_no 0 for uppoint %d\n",__FILE__,__LINE__,__FUNCTION__,station_no,phy_no);
            i++;
            continue;
        }

        int index_no;
        index_no = db_uppoi_info[i].index_no;
        if((index_no < 0) || (index_no > SimFes_stations[station_no].max_uppoi_no))
        {
            fprintf(stdin,"%s:%d:%s	[ERROR]index_no %d > max_uppoi_no %d for uppoint %d\n",__FILE__,__LINE__,__FUNCTION__,index_no,SimFes_stations[station_no].max_uppoi_no,phy_no);
            i++;
            continue;
        }

        SimFes_uppoints[j].point_id=db_uppoi_info[i].id;
        SimFes_uppoints[j].point_no=phy_no;

        SimFes_uppoints[j].station_no=station_no;
        SimFes_uppoints[j].index_no=index_no;
        SimFes_uppoints[j].psid=db_uppoi_info[i].psid;
        SimFes_stations[station_no].uppoints[index_no] = phy_no;
        SimFes_stations[station_no].uppoi_num++;
        SimFes_uppoints[j].value=0;
        SimFes_uppoints[j].status=0;
        SimFes_multi_point tmp;
        tmp.psid=SimFes_uppoints[j].psid;
        SimFes_uppoints[j].multi_point_ptr=(SimFes_multi_point *)bsearch(&tmp,SimFes_multi_uppoints,SimFes_multi_uppoint_num,sizeof(SimFes_multi_uppoints[0]),cmp_mul_poi);

        //SimFes_uppoints[j].valid=VALID;

        i++;
        j++;
#ifdef SIMFES_DEBUG
        printf("%s:%d:%s	[DEBUG]phy_no:%d,station_no:%d,point_id:%ld,point_no:%d,index_no:%d,psid:%ld\n",__FILE__,__LINE__,__FUNCTION__,phy_no,station_no, SimFes_uppoints[j].point_id,SimFes_uppoints[j].point_no, SimFes_uppoints[j].index_no,SimFes_uppoints[j].psid);
#endif
    }
    SimFes_valid_uppoint_num=j;

    if((ret_val=CloseTable(&tb_des))<0)
    {
        fprintf(stdin,"%s:%d:%s	[ERROR]close table %d failed,ret_val=%d",__FILE__,__LINE__,__FUNCTION__,SIMFES_FES_RECV_POINT_NO,ret_val);
        return -1;
    }

    return 0;
}

//功能:	按psid字段排序遥测点
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_sort_upana_by_psid(void)
{
    qsort(SimFes_upanalogs,SimFes_valid_upanalog_num,sizeof(SimFes_analog),cmp_ana);
}

//功能:	按psid字段排序遥信点
//参数:	无
//返回值:	成功>=0;
//		失败<0
int SimFes_sort_uppoi_by_psid(void)
{
    qsort(SimFes_uppoints,SimFes_valid_uppoint_num,sizeof(SimFes_point),cmp_poi);
}


//功能: 	按psid字段去除重复的遥测点，这样每个遥测点只有一个源，简化多源处理
//参数: 	无
//返回值:	去重后的遥测点数目;
//
int SimFes_uniq_upana_by_psid(void)
{
    int i,j;

    i=0;
    j=1;
    while(j<SimFes_valid_upanalog_num)
    {
        if(SimFes_upanalogs[j].psid==SimFes_upanalogs[i].psid)
        {
            j++;
        }
        else
        {
            i++;
            SimFes_upanalogs[i]=SimFes_upanalogs[j];
            j++;
        }
    }
    return ++i;
}

//功能: 	按psid字段去除重复的遥信点，这样每个遥信点只有一个源，简化多源处理
//参数: 	无
//返回值:	去重后的遥信点数目;
//
int SimFes_uniq_uppoi_by_psid(void)
{
    int i,j;

    i=0;
    j=1;
    while(j<SimFes_valid_uppoint_num)
    {
        if(SimFes_uppoints[j].psid==SimFes_uppoints[i].psid)
        {
            j++;
        }
        else
        {
            i++;
            SimFes_uppoints[i]=SimFes_uppoints[j];
            j++;
        }
    }
    return ++i;
}


//功能:	算子，比较遥测设备表中的元素
//参数:	待比较元素
//返回值:	大于-'1';
//		小于-'-1'
int cmp_ana_tab(const void *a,const void *b)
{
    if((*(SimFes_sca_ana_dev_tab *)a).valid==INVALID)
    {
        return -1;
    }
    else if((*(SimFes_sca_ana_dev_tab *)b).valid==INVALID)
    {
        return 1;
    }
    else
    {
        return (*(SimFes_sca_ana_dev_tab *)a).table_no - (*(SimFes_sca_ana_dev_tab *)b).table_no;
    }
}

//功能:	算子，比较遥信设备表中的元素
//参数:	待比较元素
//返回值:	大于-'1';
//		小于-'-1'
int cmp_poi_tab(const void *a,const void *b)
{
    if((*(SimFes_sca_poi_dev_tab *)a).valid==INVALID)
    {
        return -1;
    }
    else if((*(SimFes_sca_poi_dev_tab *)b).valid==INVALID)
    {
        return 1;
    }
    else
    {
        return (*(SimFes_sca_poi_dev_tab *)a).table_no - (*(SimFes_sca_poi_dev_tab *)b).table_no;
    }
}


//功能:	算子，按psid字段比较遥测点
//参数:	待比较元素
//返回值:	大于-'1';
//		小于-'-1'
int cmp_ana(const void *a,const void *b)
{
	if((*(SimFes_analog*)a).psid > (*(SimFes_analog*)b).psid)
	{
		return 1;
	}else if((*(SimFes_analog*)a).psid < (*(SimFes_analog*)b).psid)
	{
		return -1;
	}else
	{
		return 0;
	}

    //return (*(SimFes_analog*)a).psid - (*(SimFes_analog*)b).psid;
}

//功能:	算子，按psid字段比较多源遥信点
//参数:	待比较元素
//返回值:	大于-'1';
//		小于-'-1'
int cmp_poi(const void *a,const void *b)
{
	if((*(SimFes_point*)a).psid > (*(SimFes_point*)b).psid)
	{
		return 1;
	}else if((*(SimFes_point*)a).psid < (*(SimFes_point*)b).psid)
	{
		return -1;
	}else
	{
		return 0;
	}

    //return (*(SimFes_point*)a).psid - (*(SimFes_point*)b).psid;
}

//功能:	按设备表分割遥测量
//参数:	无
//返回值:	无
void SimFes_split_ana_by_tab(void)
{
    char time_str[32];

    qsort(SimFes_sca_ana_dev_tabs,SimFes_sca_ana_dev_tab_num,sizeof(SimFes_sca_ana_dev_tab),cmp_ana_tab);
    for(int i=0; i<SimFes_uniq_upanalog_num; i++)
    {
        long table_no;
        SimFes_sca_ana_dev_tab tmp;
        SimFes_sca_ana_dev_tab * tmp_p;

        table_no=SimFes_upanalogs[i].psid & TABLE_NO_MASK;
        table_no=table_no >> 48;
        tmp.table_no=(short)table_no;
        tmp.valid=VALID;

        if((tmp_p=(SimFes_sca_ana_dev_tab *)bsearch(&tmp,SimFes_sca_ana_dev_tabs,SimFes_sca_ana_dev_tab_num,sizeof(SimFes_sca_ana_dev_tab),cmp_ana_tab))==NULL)
        {
            fprintf(stdin,"%s:%d:%s		[WARN]ID %ld table_no %d skip\n",__FILE__,__LINE__,__FUNCTION__,SimFes_upanalogs[i].psid,tmp.table_no);
            continue;
        }
#ifdef SIMFES_DEBUG
        printf("%s:%d:%s	[DEBUG]table_no:%d,offset:%d,id:%ld,psid:%ld\n",__FILE__,__LINE__,__FUNCTION__,tmp.table_no,tmp_p->upana_num,SimFes_upanalogs[i].analog_id,SimFes_upanalogs[i].psid);
#endif
        get_time(time_str);
        fprintf(tmp_p->logfile,"%s	[DEBUG]offset:%d,id:%ld,psid:%ld\n",time_str,tmp_p->upana_num,SimFes_upanalogs[i].analog_id,SimFes_upanalogs[i].psid);
        tmp_p->upanalogs[tmp_p->upana_num]=i;
        tmp_p->upana_num++;
    }
    return;
}


//功能:	按设备表分割遥信量
//参数:	无
//返回值:	无
void SimFes_split_poi_by_tab(void)
{
    qsort(SimFes_sca_poi_dev_tabs,SimFes_sca_poi_dev_tab_num,sizeof(SimFes_sca_poi_dev_tab),cmp_poi_tab);
    for(int i=0; i<SimFes_uniq_uppoint_num; i++)
    {
        long table_no;
        SimFes_sca_poi_dev_tab tmp;
        SimFes_sca_poi_dev_tab * tmp_p;

        table_no=SimFes_uppoints[i].psid& TABLE_NO_MASK;
        table_no=table_no >> 48;
        tmp.table_no=(short)table_no;
        tmp.valid=VALID;

        if((tmp_p=(SimFes_sca_poi_dev_tab *)bsearch(&tmp,SimFes_sca_poi_dev_tabs,SimFes_sca_poi_dev_tab_num,sizeof(SimFes_sca_poi_dev_tab),cmp_poi_tab))==NULL)
        {
            fprintf(stdin,"%s:%d:%s		[WARN]ID %ld table_no %d skip\n",__FILE__,__LINE__,__FUNCTION__,SimFes_uppoints[i].point_id,tmp.table_no);
            continue;
        }
#ifdef SIMFES_DEBUG
        printf("%s:%d:%s	[DEBUG]table_no:%d,offset:%d,id:%ld,psid:%ld\n",__FILE__,__LINE__,__FUNCTION__,tmp.table_no,tmp_p->uppoi_num,SimFes_uppoints[i].point_id,SimFes_uppoints[i].psid);
#endif
        tmp_p->uppoints[tmp_p->uppoi_num]=i;
        tmp_p->uppoi_num++;
    }
    return;
}


//功能:	获取当前时间
//参数:	缓冲区
//返回值:	无
void get_time(char *time_str)
{
    char timestamp[16];
    struct tm *tm;
    time_t now;
    now=time(NULL);
    tm=localtime(&now);
    sprintf(time_str,"%d-%02d-%02d %02d:%02d:%02d",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
    return;
}
int analog_is_exist(long psid){
	long key_no;
	int field_no,table_no,ret_code;
	float val;
	TB_DESCR tb_des;
	key_no = psid & 0xffff0000ffffffff;
	field_no = (short)(psid>>32);
	table_no = (int)(psid>>48);
	ret_code=0;
	if (   OpenTableByID(NULL,AC_REALTIME_NO,AP_SCADA,table_no,&tb_des)==0 
		&& GetFieldsByID(&tb_des,(char*)&key_no,&field_no,1,sizeof(val),(char*)&val)==0){
		ret_code=1;
	}
	CloseTable(&tb_des);
	return ret_code;
	
}
