//2012年 05月 05日 星期六 00:37:47 CST
//written by ranpanf
//test of reading some table of rtdb
#include "db_api/rtdb_api.h"
#include "db_struct_fes.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int analog_is_exist(long psid);
int main(){
	FILE *logfile;
	TB_DESCR tb_des;
	RECV_ANALOG_FES*recs_buff;
	int recs_buff_size;
	int num,i,j,tabnos[100],tabnoscnt,tabno,valid_cnt;
	bzero(&tabnos,sizeof(tabnos));
	
	if ( OpenTableByName(NULL,"realtime","fes","recv_analog",&tb_des)<0 ){
		fprintf(stderr,"Error:Table Can't Be Opened!\n");
		exit(1);
	}

	if ( ( recs_buff_size=GetTableRecs(&tb_des,0,(char**)&recs_buff))<0){
		fprintf(stderr,"Error:Failing in Geting all Records!\n");
		exit(1);
	}

	num = recs_buff_size/sizeof(RECV_ANALOG_FES);
	tabnoscnt=0;
	valid_cnt=0;
	for ( i=0;i<num;i++){
		if(analog_is_exist(recs_buff[i].psid))
			valid_cnt++;
		tabno=recs_buff[i].psid>>48;
		for( j=0;j<tabnoscnt&&tabnos[j]!=tabno;j++);
		if(j==tabnoscnt)tabnos[tabnoscnt++]=tabno;
	}
	printf("num=%d\n",num);
	printf("valid_cnt=%d\n",valid_cnt);
	printf("tabnoscnt=%d\n",tabnoscnt);
	for(j=0;j<tabnoscnt;j++){
		printf("%d\t",tabnos[j]);
	}
	printf("\n");
	CloseTable(&tb_des);
	return 0;
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
