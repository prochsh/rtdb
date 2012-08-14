#include "Simulate_Fes.hxx"
#include "Simulate_Fes_queue.hxx"
#include <d5000.hxx>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

extern SIMFES_QUEUE* SimFes_ana_fifo;
extern SimFes_sca_ana_dev_tab *SimFes_sca_ana_dev_tabs;
extern SimFes_analog *SimFes_upanalogs;
extern int SimFes_sca_ana_dev_tab_num;
extern int SimFes_ana_chg_interval;
#define MAX_FILE_NAME_LEN	64
#define MAX_CMD_STR_LEN	128
#define MAX_PARA_STR_LEN	128
#define RIGHT	0
#define WRONG	-1
#define SAMPLE_INTEVAL	20
#define INVALID -1
#define MAXLINE 64

float SimFes_ana_next_value(int i);
void SimFes_ana_check_sample_value(int i);
int SimFes_ana_list_by_tab(int i,char *input_file,char *timestamp);

//功能: 	模拟遥测量变化行为
//参数: 	无
//返回值:	无
//备注:	可扩展压力测试
void *SimFes_ana_action(void *arg)
{
	while(1){

		for(int i=0; i<SimFes_sca_ana_dev_tab_num; i++)
		{
			if(SimFes_sca_ana_dev_tabs[i].valid==INVALID)
				continue;

			for(int j=0; j<SimFes_sca_ana_dev_tabs[i].upana_num; j++)
			{
				int index;
				SimFes_multi_analog *multi_analog_ptr;

				index=SimFes_sca_ana_dev_tabs[i].upanalogs[j];
				multi_analog_ptr=SimFes_upanalogs[index].multi_analog_ptr;

				if(multi_analog_ptr)
				{
					float raw_value;
					SGDPS_CALGCHG chg_ana;

					raw_value=SimFes_ana_next_value(index);
					if(multi_analog_ptr->value!=raw_value)
					{
						multi_analog_ptr->value=raw_value;
					}
					chg_ana.keyid=multi_analog_ptr->psid;
					chg_ana.value=multi_analog_ptr->value;

					struct timeval tv;
					gettimeofday (&tv,NULL);
					struct tm *tm;
					tm=localtime(&tv.tv_sec);

					fprintf(SimFes_sca_ana_dev_tabs[i].logfile,
							"%d-%d-%d %d:%d:%d:%d	psid=%ld\tvalue=%f\n",
							tm->tm_year+1900,
							tm->tm_mon,
							tm->tm_mday,
							tm->tm_hour,
							tm->tm_min,
							tm->tm_sec,
							tv.tv_usec/1000,
							chg_ana.keyid,
							chg_ana.value);

					SimFes_queue_add(SimFes_ana_fifo,(void *)&chg_ana);
					//睡眠，单位微妙
					usleep(SimFes_ana_chg_interval);
				}
			}
			//等待遥测值被采样进HBase
			//sleep(SAMPLE_INTEVAL);
			//SimFes_ana_check_sample_value(i);
		}
	}
	return NULL;
}

//功能: 	产生下一个遥测值
//参数: 	遥测量下标
//返回值:	遥测值
//备注:	可扩展遥测值生成方式，例如从断面中读取遥测值
float SimFes_ana_next_value(int i)
{
    int range;
	timeval tv;
    float min_raw_value,max_raw_value,raw_value,random;
    min_raw_value=SimFes_upanalogs[i].min_raw_value;
    max_raw_value=SimFes_upanalogs[i].max_raw_value;

	gettimeofday(&tv,NULL);
    range=(int)max_raw_value-(int)min_raw_value;
    srand((unsigned)(tv.tv_sec+tv.tv_usec));
    random=(float)rand()/RAND_MAX;
    raw_value=random*range+min_raw_value;
    SimFes_upanalogs[i].raw_value=raw_value;

    return raw_value;
}

//功能: 	检查采样的HBase中的遥测值是否正确
//参数: 	表下标
//返回值:	无
void SimFes_ana_check_sample_value(int i)
{
    int table_no=SimFes_sca_ana_dev_tabs[i].table_no;

    char *d5000_home;
    if((d5000_home=getenv("D5000_HOME"))==NULL)
    {
        fprintf(stdin,"%s:%d:%s		[ERROR]getenv $D5000_HOME failed\n",__FILE__,__LINE__,__FUNCTION__);
        return;
    }

    char timestamp[16];
    struct tm *tm;
    time_t now;
    now=time(NULL);
    tm=localtime(&now);
    sprintf(timestamp,"%d%02d%02d%02d%02d%02d",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,0);

    char proc_log_dir[MAX_FILE_NAME_LEN];
    bzero(proc_log_dir,sizeof(proc_log_dir));
    sprintf(proc_log_dir,"%s/var/log/fes/fes_simu/",d5000_home);

    char input_file[MAX_FILE_NAME_LEN];
    bzero(input_file,sizeof(input_file));
    sprintf(input_file,"%s%d-%s",proc_log_dir,table_no,timestamp);

    char output_file[MAX_FILE_NAME_LEN];
    bzero(output_file,sizeof(output_file));
    sprintf(output_file,"%s.out",input_file);

    //从hbase读取记录的id列表
    if(SimFes_ana_list_by_tab(i,input_file,timestamp)<0)
    {
        fprintf(stdin,"%s:%d:%s		[WARN]input analog list failed for %d\n",__FILE__,__LINE__,__FUNCTION__,i);
        return;
    }

    //调用fes_simu_read_hbase.sh获取hbase中的采样值，保存为文件
    char cmdstring[MAX_CMD_STR_LEN];
    char para1[MAX_PARA_STR_LEN],para2[MAX_PARA_STR_LEN];
    strncpy(para1,input_file,strlen(input_file));
    strncpy(para2,proc_log_dir,strlen(proc_log_dir));
    sprintf(cmdstring,"%s/bin/fes_simu_read_hbase.sh %s %s",d5000_home,para1,para2);
	system(cmdstring);

    //逐个比对hbase中的采样值
    FILE *fp;
    if((fp=fopen(output_file,"r"))==NULL)
    {
        fprintf(stdin,"%s:%d:%s		[ERROR]open %s failed\n",__FILE__,__LINE__,__FUNCTION__,output_file);
        return;
    }

    char buf[MAXLINE];
    int nline=0;
    while(fgets(buf,MAXLINE,fp)!=NULL)
    {
        if(nline>SimFes_sca_ana_dev_tabs[i].upana_num)
        {
            fprintf(stdin,"%s:%d:%s		[ERROR]lines %d > upana_num %d,skip the remained lines\n",__FILE__,__LINE__,__FUNCTION__,nline,SimFes_sca_ana_dev_tabs[i].upana_num);
            break;
        }
        int index=SimFes_sca_ana_dev_tabs[i].upanalogs[nline];

        char *key;
        char *value;

        key=strtok(buf," ");
        value=strtok(NULL,"\n");

        if(SimFes_upanalogs[index].psid!=atoi(key))
        {
            fprintf(stdin,"%s:%d:%s		[ERROR]key not match:%ld\t%ld\n",__FILE__,__LINE__,__FUNCTION__,key,SimFes_upanalogs[index].analog_id);
            continue;
        }

        //会有误差吗?
        if(SimFes_upanalogs[index].raw_value==strtof(value,NULL))
        {
            SimFes_upanalogs[index].flag=RIGHT;
        }
        else
        {
            SimFes_upanalogs[index].flag=WRONG;
            SimFes_sca_ana_dev_tabs[i].error++;
        }

        nline++;
    }

    if(ferror(fp))
    {
        fprintf(stdin,"%s:%d:%s		[ERROR]read %s failed\n",__FILE__,__LINE__,__FUNCTION__,output_file);
        return;
    }
}

//功能: 	列出每个设备表中所有遥测量的id，id已递增排好序
//参数: 	表下标
//返回值:	成功-遥测量的数目
//		失败-'-1'
int SimFes_ana_list_by_tab(int i,char *input_file,char *timestamp)
{
    short table_no;
    table_no=SimFes_sca_ana_dev_tabs[i].table_no;

    FILE *fp;
    if((fp=fopen(input_file,"w"))==NULL)
    {
        fprintf(stdin,"%s:%d:%s		[ERROR]open %s failed\n",__FILE__,__LINE__,__FUNCTION__,input_file);
        return -1;
    }

    fprintf(fp,"%s\n",timestamp);
	fprintf(fp,"%d\n",table_no);
    
	int j;
    for(j=0; j<SimFes_sca_ana_dev_tabs[i].upana_num; j++)
    {
        fprintf(fp,"%ld\n",SimFes_upanalogs[SimFes_sca_ana_dev_tabs[i].upanalogs[j]].psid);
    }
    fclose(fp);

    return j;
}
