//2012年 05月 07日 星期一 07:08:54 CST
//last modified:2012年 05月 08日 星期二 05:37:08 CST
//written by ranpanf
//access rtdb by C++ interface
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdlib>
#include "db_api/odb_tableop.h"
#include "db_struct_fes.h"
#include "sys/time.h"
using namespace ODB;
#define TEST_POINT_NUM 10000
int main(){
	
	FILE *logfile;
	CTableOp tabop;
	MULTI_ANALOG_FES * bufptr;
	int effect_tstpt_num;
	int bufsz;
	int j,i,ret_code;
	int table_no,field_no,key_no;
	long key;
	char table_name[64],field_name[64],key_name[64];
	float val;
    struct timeval tv1,tv2; 
	// measure points.
	struct {
		union {
			long psid;
			struct{
				int   key_no;
				short field_no;
				short table_no;
			}id;
		}tstpt;
		int tstcnt;
		int effect_tstcnt;
		int max_interval;
		int min_interval;
		float oldval;
		float val;
	}tstdata[TEST_POINT_NUM];
    
	bzero(tstdata,sizeof (tstdata));

	if ( (logfile=fopen("log1","w"))==NULL ){
		perror("logfile open(fopen)");
		exit(1);
	}

    // table name:multi_analog
	// table no  :1218
	// dbi:"d5000"->"FES"->"定义表类"->"多源遥测信息表"
/*	if ( tabop.Open("fes","multi_analog")!=DB_OK ){
		fprintf(stderr,"Error:Table Can't Be Opened!\n");
		exit(1);
	}
*/
	if ( (ret_code=tabop.TableGet((char**)&bufptr,bufsz))<0){
		fprintf(stderr,"Error:Failing in Geting all Records!\n:return:%d\n",ret_code);
		exit(1);
	}

	// table "multi_analog" has effect_tstpt_num" tst point.
    effect_tstpt_num = bufsz /sizeof (MULTI_ANALOG_FES);
	// just choose first serveral points to tst.
	effect_tstpt_num = (effect_tstpt_num < TEST_POINT_NUM ?
			             effect_tstpt_num:
						TEST_POINT_NUM);

    //read tst points' keys
	for (i=0;i<effect_tstpt_num;i++){
		tstdata[i].tstpt.psid=bufptr[i].psid;
	}
	//tabop.Close();
#if 1
	j=0;
	while (j++<1){
		
		gettimeofday(&tv1,NULL);
		for (i=0;i<effect_tstpt_num;i++){
			// open table
			table_no=tstdata[i].tstpt.id.table_no;
			field_no=tstdata[i].tstpt.id.field_no;
			key_no=tstdata[i].tstpt.id.key_no;
/*			if ( tabop.Open("scada",table_no)!=DB_OK){
				fprintf(stderr,"Error:Table(%d) Can not be Opened!\n",table_no);
				continue;
			}
*/
			key=tstdata[i].tstpt.psid&0xFFFF0000FFFFFFFF;
            if ( tabop.TableGetByKey((char*)&key,field_no,(char*)&val,sizeof(val))<0){
				fprintf(stderr,"Error:Value(%d,%d,%d) Can not be Get!\n",table_no,key_no,field_no);
				fprintf(stderr,"psid=%ld\n",tstdata[i].tstpt.psid);
				continue;
			}

            if ( tabop.GetTableNameByNo(table_name,table_no)<0){
				fprintf(stderr,"Error:Table(%d)Name Can not be Get!\n",table_no);
				continue;
			}
			if ( tabop.TableGetByKey((char*)&key,2,(char*)&key_name,sizeof(key_name))<0){
				fprintf(stderr,"Error:Table Record(%d,%d)Name Can not be Get!\n",table_no,key_no);
				continue;
			}
			if ( tabop.GetFieldNameByNo(field_name,field_no)<0){
				fprintf(stderr,"Error:Table Record Field(%d,%d,%d)Name Can not be Get!\n",table_no,key_no,field_no);
				continue;
			}
			printf("Table:%s\nRecord:%s\nField:%s\n",table_name,key_name,field_name);
			printf("Value(%d,%d,%d)=%.1f\n",table_no,key_no,field_no,val);
		//	tabop.Close();

		}
		gettimeofday(&tv2,NULL);
		printf("Take %dms time\n",(tv2.tv_sec - tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000);
	}
#endif
	return 0;
}


