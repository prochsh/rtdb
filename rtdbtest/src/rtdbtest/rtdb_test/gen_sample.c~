#include"rtdb_test.h"
#include<system.h>
#include<db_api/odb_common.h>
#include<db_api/odb_define.h>
#include<db_api/odb_tableop.h>
#include<db_api/odb_struct.h>

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>

using namespace std;
#define TABLE_FIELD_MAX_NUM 128
TABLE_FIELD table_field[TABLE_FIELD_MAX_NUM];
int table_field_num;


TEST_POINT*samp_pts;

int config_table_field(TABLE_FIELD*tbl_fld,FILE*fp);
void print_table_field(TABLE_FIELD*,int);
int main(int argc,char*argv[])
{
	int samp_pts_num
		,samp_fd
		,i,j;
	FILE *fconf;
	KEY_STRU key_stru;

	samp_pts_num=atoi(argv[1]);

	if ( (samp_fd =open("sampfile",O_RDWR|O_CREAT|O_TRUNC,777))<0){ 
		perror("##[ERROR]fail in openning file 'sampfile'");
		exit(1);
	}
	samp_pts=(TEST_POINT*) mmap( NULL,sizeof(TEST_POINT)*samp_pts_num
			                   , PROT_READ|PROT_WRITE,MAP_PRIVATE,samp_fd,0);
	if (samp_pts == NULL) {
		perror("###[ERROR]fail in mmapping file 'samp_fd'");
		exit(1);
	}
	fconf=fopen("gen_sample.conf","r");
	table_field_num=config_table_field(table_field,fconf);
	//print_table_field(table_field,table_field_num);
	
	for (i=0;i<samp_pts_num;i++) {
		key_stru.table_no=table_field[i%table_field_num].table_no;
		key_stru.field_id=table_field[i%table_field_num].field_no;

	}
	return 0;





}
int config_table_field(TABLE_FIELD*tbl_fld,FILE*fp)
{
	char line[64],*token,*saveptr;
	int  i;
	i=0;
	while ( fgets(line,64,fp)!= NULL ){
		if (i>=TABLE_FIELD_MAX_NUM){
			printf("##[WARNING]table_fields fail in holding more configure infos");
			return i;
		}
		if ( *line=='#') continue;
		token=strtok_r(line,":",&saveptr);
		tbl_fld[i].table_no=atoi(token);
		token=strtok_r(NULL,"\n",&saveptr);
		tbl_fld[i].field_no=atoi(token);
		i++;
	}
	return  i;
}
void print_table_field(TABLE_FIELD*tbl_fld,int num)
{
	int i;
	printf("table\tfield\n");
	for (i=0;i<num;i++) {
		printf("%d[%d\t%d]\n",i,table_field[i].table_no,table_field[i].field_no);
	}
}
