#include "rtdb_test.h"

#define test_insert 300
#define num_op 8

#define num_test 1
#define KEY_VALUE 41

#define SUM_NUM 10000
#define interval_op 2
#define interval_insert 200000
#define random_up 30000

unsigned long kid_test= 0;
unsigned long kid = 0;
int table_id = 5507;
FILE *fp_ret=NULL;
FILE *fp_time=NULL;
	
KEY_STRU key_stru;


unsigned int size=520;
unsigned long num=0;
int index_test=0;

float time_use[num_op][SUM_NUM/interval_op][num_test]={0};
float time_use_print[num_op][SUM_NUM/interval_op][3]={0};

float time_use1[num_op][SUM_NUM/interval_op][num_test]={0};
float time_use_print1[num_op][SUM_NUM/interval_op][3]={0};

float time_insert[test_insert]={0};

float time_pingjun[num_op];

int insert_record(TB_DESCR* ptb_des)
	{
		int ret_code=0;
	
		struct timeval tpstart,tpend,tpnow; 
		float timeuse=0; 
		unsigned int seedVal=0;
		struct testdata test_data;

		gettimeofday(&tpnow,NULL);
		
    		seedVal=((((unsigned int)tpnow.tv_sec&0xFFFF)+
               (unsigned int)tpnow.tv_usec)^
               (unsigned int)tpnow.tv_usec);
			
   		 srand((unsigned int)seedVal);
				
		sprintf(test_data.ename,"test.code.%ld",num+1);
		sprintf(test_data.cname,"测试. 湖南.株洲新厂3G负旋转备用容量/计算量.%ld",num+1);
		test_data.id=kid;	

		test_data.flags=(int)rand()%random_up;
		test_data.ori_value=(rand()/(double)(RAND_MAX))*random_up;
		test_data.valid_low=(rand()/(double)(RAND_MAX))*random_up;
		test_data.valid_up=(rand()/(double)(RAND_MAX))*random_up;

		gettimeofday(&tpstart,NULL); 
		
		ret_code = InsertRec(ptb_des, 1,  size, (char*)&test_data, 1);

		gettimeofday(&tpend,NULL); 

		
		if(ret_code<0)
		{
			cout<<"X";
			return -1;
		}

	if(num%interval_insert==0)
	{	
		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
		tpend.tv_usec-tpstart.tv_usec; 
		timeuse/=1000000; 	
		time_insert[num/interval_insert]=timeuse;
	}

	return 0;
	
	}

//测试读全表
void* test_GetTableRecs(void* ptr )
{
	struct timeval tpstart,tpend; 

	struct timeval tpstart1,tpend1; 
	
	float timeuse=0; 
	char* buf_ptr = NULL;
	TB_DESCR tb_des;
	int ret_open,ret_close=0;
	int ret_code=0;
	
	gettimeofday(&tpstart,NULL);
	
	ret_open = OpenTableByID(NULL, 1, 100000, table_id,&tb_des);

	gettimeofday(&tpstart1,NULL);
	
	ret_code = GetTableRecs(&tb_des, 0, &buf_ptr);
	
	gettimeofday(&tpend1,NULL); 

	ret_close = CloseTable(&tb_des);

	gettimeofday(&tpend,NULL); 

	//time_use
	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
	tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=1000000; 
	cout<<"GetTableRecs  "<<index_test<<"  time="<<timeuse<<endl;
	time_use[0][int(num/(interval_op))][index_test]=timeuse;

	//time_use1
	timeuse=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+ 
	tpend1.tv_usec-tpstart1.tv_usec; 
	timeuse/=1000000; 
	cout<<"GetTableRecs  "<<index_test<<"  time1="<<timeuse<<endl;
	time_use1[0][int(num/(interval_op))][index_test++]=timeuse;
	
	fprintf(fp_ret,"打开表返回值%d\n",ret_open);
	fprintf(fp_ret,"GetTableRecs返回值%d\n",ret_code);
	fprintf(fp_ret,"关闭表返回值%d\n",ret_close);	
	if(ret_open < 0)
		fprintf(fp_ret,"打开表 %d错误，错误号=%d\n",table_id,ret_open);
	
	if(ret_code<0)
		fprintf(fp_ret,"GetTableRecs  error\n" );	
	
	if(ret_close < 0)
		fprintf(fp_ret,"关闭表 %d错误，错误号=%d\n",table_id,ret_close);
	
	BufFree(buf_ptr);
	return ptr;
}

//读全表的7列
void* test_GetTableFields(void* ptr )
{
	struct timeval tpstart,tpend; 
	struct timeval tpstart1,tpend1; 
	float timeuse=0; 
	char* buf_ptr = NULL;
	TB_DESCR tb_des;
	int ret_open,ret_close=0;
	int ret_code=0;

	struct id_code_name_stru *id_code_name_ptr;
	int field_length;
	int field_array_ptr[7]={1,2,3,6,7,8,9};
	int data_size =7;

	gettimeofday(&tpstart,NULL);
	
	ret_open = OpenTableByID(NULL, 1, 100000, table_id,&tb_des);	

	gettimeofday(&tpstart1,NULL);
	
	ret_code=GetTableFields(&tb_des, 0, field_array_ptr, data_size, (char **)&id_code_name_ptr);

	gettimeofday(&tpend1,NULL); 

	ret_close = CloseTable(&tb_des);

	gettimeofday(&tpend,NULL); 

	
	//time_use
	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
	tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_GetTableFields  "<<index_test<<"  time="<<timeuse<<endl;
	time_use[1][int(num/(interval_op))][index_test]=timeuse;

	//time_use1
	timeuse=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+ 
	tpend1.tv_usec-tpstart1.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_GetTableFields  "<<index_test<<"  time1="<<timeuse<<endl;
	time_use1[1][int(num/(interval_op))][index_test++]=timeuse;
	
	fprintf(fp_ret,"打开表返回值%d\n",ret_open);
	fprintf(fp_ret,"test_GetTableFields返回值%d\n",ret_code);
	fprintf(fp_ret,"关闭表返回值%d\n",ret_close);	
	if(ret_open < 0)
		fprintf(fp_ret,"打开表 %d错误，错误号=%d\n",table_id,ret_open);
	
	if(ret_code<0)
		fprintf(fp_ret,"test_GetTableFields  error\n" );
	
	if(ret_close < 0)
		fprintf(fp_ret,"关闭表 %d错误，错误号=%d\n",table_id,ret_close);

	BufFree(buf_ptr);
	return ptr;
}

//11．读一条记录（通过名称）
void* test_GetRecByName_string(void* ptr )
{
	struct timeval tpstart,tpend; 
	struct timeval tpstart1,tpend1; 
	float timeuse=0; 
	char* buf_ptr = NULL;
	TB_DESCR tb_des;
	int ret_open,ret_close=0;
	int ret_code=0;
	struct testdata test_data;
	char cname[64]="山东.博山站/220kV.白山线/电流值";
	
	
	gettimeofday(&tpstart,NULL);
	
	ret_open = OpenTableByID(NULL, 1, 100000, table_id,&tb_des);

	gettimeofday(&tpstart1,NULL);

	ret_code = GetRecByName(&tb_des ,cname, sizeof(testdata), (char*)&test_data);

	gettimeofday(&tpend1,NULL); 

	ret_close = CloseTable(&tb_des);

	gettimeofday(&tpend,NULL); 
	
	//time_use
	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
	tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_GetRecByName_string  "<<index_test<<"  time="<<timeuse<<endl;
	time_use[2][int(num/(interval_op))][index_test]=timeuse;

	//time_use1
	timeuse=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+ 
	tpend1.tv_usec-tpstart1.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_GetRecByName_string  "<<index_test<<"  time1="<<timeuse<<endl;
	time_use1[2][int(num/(interval_op))][index_test++]=timeuse;
	
	fprintf(fp_ret,"打开表返回值%d\n",ret_open);
	fprintf(fp_ret,"test_GetRecByName_string返回值%d\n",ret_code);
	fprintf(fp_ret,"关闭表返回值%d\n",ret_close);	
	if(ret_open < 0)
		fprintf(fp_ret,"打开表 %d错误，错误号=%d\n",table_id,ret_open);
	
	if(ret_code<0)
		fprintf(fp_ret,"test_GetRecByName_string  error\n" );
	
	if(ret_close < 0)
		fprintf(fp_ret,"关闭表 %d错误，错误号=%d\n",table_id,ret_close);
	
	BufFree(buf_ptr);
	return ptr;
}

//11．读一条记录（通过名称）
void* test_GetRecByName_float(void* ptr )
{
	struct timeval tpstart,tpend; 
	struct timeval tpstart1,tpend1; 
	float timeuse=0; 
	char* buf_ptr = NULL;
	TB_DESCR tb_des;
	int ret_open,ret_close=0;
	int ret_code=0;
	struct testdata test_data;
	
	gettimeofday(&tpstart,NULL);
	
	ret_open = OpenTableByID(NULL, 1, 100000, table_id,&tb_des);

	gettimeofday(&tpstart1,NULL);

	ret_code = GetRecByName(&tb_des ,test_data.cname, sizeof(testdata), (char*)&test_data);

	gettimeofday(&tpend1,NULL); 

	ret_close = CloseTable(&tb_des);

	gettimeofday(&tpend,NULL); 
	
	//time_use
	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
	tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_GetRecByName_float  "<<index_test<<"  time="<<timeuse<<endl;
	time_use[3][int(num/(interval_op))][index_test]=timeuse;

	//time_use1
	timeuse=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+ 
	tpend1.tv_usec-tpstart1.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_GetRecByName_float  "<<index_test<<"  time1="<<timeuse<<endl;
	time_use1[3][int(num/(interval_op))][index_test++]=timeuse;
	
	fprintf(fp_ret,"打开表返回值%d\n",ret_open);
	fprintf(fp_ret,"test_GetRecByName_float返回值%d\n",ret_code);
	fprintf(fp_ret,"关闭表返回值%d\n",ret_close);	
	if(ret_open < 0)
		fprintf(fp_ret,"打开表 %d错误，错误号=%d\n",table_id,ret_open);
	
	if(ret_code<0)
		fprintf(fp_ret,"test_GetRecByName_float  error\n" );
		
	if(ret_close < 0)
		fprintf(fp_ret,"关闭表 %d错误，错误号=%d\n",table_id,ret_close);
	
	BufFree(buf_ptr);
	return ptr;
}

//8．读一条记录（通过关键字）
void* test_GetRecByID(void* ptr )
{
	struct timeval tpstart,tpend; 
	struct timeval tpstart1,tpend1; 
	float timeuse=0; 
	char* buf_ptr = NULL;
	TB_DESCR tb_des;
	int ret_open,ret_close=0;
	int ret_code=0;
	struct testdata test_data;
	
	gettimeofday(&tpstart,NULL);
	
	ret_open = OpenTableByID(NULL, 1, 100000, table_id,&tb_des);

	gettimeofday(&tpstart1,NULL);

	ret_code = GetRecByID(&tb_des ,(char*)&kid_test, sizeof(testdata), (char*)&test_data);

	gettimeofday(&tpend1,NULL); 

	ret_close = CloseTable(&tb_des);

	gettimeofday(&tpend,NULL); 
	
	//time_use
	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
	tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_GetRecByID  "<<index_test<<"  time="<<timeuse<<endl;
	time_use[4][int(num/(interval_op))][index_test]=timeuse;

	//time_use1
	timeuse=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+ 
	tpend1.tv_usec-tpstart1.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_GetRecByID  "<<index_test<<"  time1="<<timeuse<<endl;
	time_use1[4][int(num/(interval_op))][index_test++]=timeuse;
	
	fprintf(fp_ret,"打开表返回值%d\n",ret_open);
	fprintf(fp_ret,"test_GetRecByID返回值%d\n",ret_code);
	fprintf(fp_ret,"关闭表返回值%d\n",ret_close);	
	if(ret_open < 0)
		fprintf(fp_ret,"打开表 %d错误，错误号=%d\n",table_id,ret_open);
	
	if(ret_code<0)
		fprintf(fp_ret,"test_GetRecByID  error\n" );
	
	if(ret_close < 0)
		fprintf(fp_ret,"关闭表 %d错误，错误号=%d\n",table_id,ret_close);
	
	BufFree(buf_ptr);
	return ptr;
}

//13．写一条记录（通过关键字）
void* test_UpdateRecByID (void* ptr )
{
	struct timeval tpstart,tpend; 
	struct timeval tpstart1,tpend1; 
	float timeuse=0; 
	char* buf_ptr = NULL;
	TB_DESCR tb_des;
	int ret_open,ret_close=0;
	int ret_code=0;
	struct testdata test_data;

	sprintf(test_data.ename,"test_UpdateRecByID %d",index_test+1);
	sprintf(test_data.cname,"test_UpdateRecByID %d",index_test+1);
	
	gettimeofday(&tpstart,NULL);
	
	ret_open = OpenTableByID(NULL, 1, 100000, table_id,&tb_des);

	gettimeofday(&tpstart1,NULL);

	ret_code = UpdateRecByID(&tb_des ,(char*)&kid_test, sizeof(testdata), (char*)&test_data);

	gettimeofday(&tpend1,NULL); 

	ret_close = CloseTable(&tb_des);

	gettimeofday(&tpend,NULL); 
	
	//time_use
	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
	tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_UpdateRecByID  "<<index_test<<"  time="<<timeuse<<endl;
	time_use[5][int(num/(interval_op))][index_test]=timeuse;

	//time_use1
	timeuse=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+ 
	tpend1.tv_usec-tpstart1.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_UpdateRecByID  "<<index_test<<"  time1="<<timeuse<<endl;
	time_use1[5][int(num/(interval_op))][index_test++]=timeuse;
	
	fprintf(fp_ret,"打开表返回值%d\n",ret_open);
	fprintf(fp_ret,"test_UpdateRecByID返回值%d\n",ret_code);
	fprintf(fp_ret,"关闭表返回值%d\n",ret_close);	
	if(ret_open < 0)
		fprintf(fp_ret,"打开表 %d错误，错误号=%d\n",table_id,ret_open);
	
	if(ret_code<0)
		fprintf(fp_ret,"test_UpdateRecByID  error\n" );
	
	if(ret_close < 0)
		fprintf(fp_ret,"关闭表 %d错误，错误号=%d\n",table_id,ret_close);
	
	BufFree(buf_ptr);
	return ptr;
}

//28．按属性读记录的一列或多列
void* test_GetRecsByAttr (void* ptr )
{
	struct timeval tpstart,tpend; 
	struct timeval tpstart1,tpend1; 
	float timeuse=0; 
	char* buf_ptr = NULL;
	TB_DESCR tb_des;
	int ret_open,ret_close=0;
	int ret_code=0;
	struct testdata test_data;
	
	int get_field_id = 2;
	int con_field_id = 1;
	int con_type = EQU;//共有6种操作
	int mask_value = 0;
	
	gettimeofday(&tpstart,NULL);
	
	ret_open = OpenTableByID(NULL, 1, 100000, table_id,&tb_des);

	gettimeofday(&tpstart1,NULL);

	ret_code = GetRecsByAttr(&tb_des, con_field_id, con_type, (char*)&kid_test, mask_value, &buf_ptr);

	gettimeofday(&tpend1,NULL); 

	ret_close = CloseTable(&tb_des);

	gettimeofday(&tpend,NULL); 
	
	//time_use
	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
	tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_GetRecsByAttr  "<<index_test<<"  time="<<timeuse<<endl;
	time_use[6][int(num/(interval_op))][index_test]=timeuse;

	//time_use1
	timeuse=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+ 
	tpend1.tv_usec-tpstart1.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_GetRecsByAttr  "<<index_test<<"  time1="<<timeuse<<endl;
	time_use1[6][int(num/(interval_op))][index_test++]=timeuse;
	
	fprintf(fp_ret,"打开表返回值%d\n",ret_open);
	fprintf(fp_ret,"test_GetRecsByAttr返回值%d\n",ret_code);
	fprintf(fp_ret,"关闭表返回值%d\n",ret_close);	
	if(ret_open < 0)
		fprintf(fp_ret,"打开表 %d错误，错误号=%d\n",table_id,ret_open);
	
	if(ret_code<0)
		fprintf(fp_ret,"test_GetRecsByAttr  error\n" );
	
	if(ret_close < 0)
		fprintf(fp_ret,"关闭表 %d错误，错误号=%d\n",table_id,ret_close);
	
	BufFree(buf_ptr);
	return ptr;
}

void test_update_CtableOp()
{
	CTableOp m_DevOp;
	
	struct timeval tpstart,tpend; 
	struct timeval tpstart1,tpend1; 
	float timeuse=0; 
	char* buf_ptr = NULL;
	//TB_DESCR tb_des;
	int ret_open,ret_close=0;
	int ret_code=0;
	struct testdata test_data;

	sprintf(test_data.ename,"test_CTableOp_UpdateRecByID %d",index_test+1);
	sprintf(test_data.cname,"test_CTableOp_UpdateRecByID %d",index_test+1);
	
	gettimeofday(&tpstart,NULL);
	
	//ret_open = OpenTableByID(NULL, 1, 100000, table_id,&tb_des);
	m_DevOp.Open(100000, 5507, 1, 1);
	
	gettimeofday(&tpstart1,NULL);

	//ret_code = UpdateRecByID(&tb_des ,(char*)&kid_test, sizeof(testdata), (char*)&test_data);
	m_DevOp.TableModifyByKey((char*)&kid_test, 2, (char*)test_data.ename , 32);

	gettimeofday(&tpend1,NULL); 

	//ret_close = CloseTable(&tb_des);

	gettimeofday(&tpend,NULL); 
	
	//time_use
	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
	tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_UpdateRecByID  "<<index_test<<"  time="<<timeuse<<endl;
	time_use[7][int(num/(interval_op))][index_test]=timeuse;

	//time_use1
	timeuse=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+ 
	tpend1.tv_usec-tpstart1.tv_usec; 
	timeuse/=1000000; 
	cout<<"test_UpdateRecByID  "<<index_test<<"  time1="<<timeuse<<endl;
	time_use1[7][int(num/(interval_op))][index_test++]=timeuse;
	
	fprintf(fp_ret,"打开表返回值%d\n",ret_open);
	fprintf(fp_ret,"test_UpdateRecByID返回值%d\n",ret_code);
	fprintf(fp_ret,"关闭表返回值%d\n",ret_close);	
	if(ret_open < 0)
		fprintf(fp_ret,"打开表 %d错误，错误号=%d\n",table_id,ret_open);
	
	if(ret_code<0)
		fprintf(fp_ret,"test_UpdateRecByID  error\n" );
	
	if(ret_close < 0)
		fprintf(fp_ret,"关闭表 %d错误，错误号=%d\n",table_id,ret_close);
	
	BufFree(buf_ptr);
	
}


void test_all()
{
	pthread_t id[num_test];
	
	for(int i=0;i<num_test;i++)
	{
		int ret=pthread_create(&(id[i]),NULL,test_GetTableRecs,NULL);
		if(ret!=0)
		{
			fprintf(fp_ret,"创建线程错误");
			exit(1);
		}
	}

	for(int i=0;i<num_test;i++)
		pthread_join(id[i],NULL);

	index_test=0;

	for(int i=0;i<num_test;i++)
	{
		int ret=pthread_create(&(id[i]),NULL,test_GetTableFields,NULL);
		if(ret!=0)
		{
			fprintf(fp_ret,"创建线程错误");
			exit(1);
		}
	}

	for(int i=0;i<num_test;i++)
		pthread_join(id[i],NULL);

	index_test=0;
/*
	for(int i=0;i<num_test;i++)
	{
		int ret=pthread_create(&(id[i]),NULL,test_GetRecByName_string,NULL);
		if(ret!=0)
		{
			fprintf(fp_ret,"创建线程错误");
			exit(1);
		}
	}

	for(int i=0;i<num_test;i++)
		pthread_join(id[i],NULL);

	index_test=0;
	
	for(int i=0;i<num_test;i++)
	{
		int ret=pthread_create(&(id[i]),NULL,test_GetRecByName_float,NULL);
		if(ret!=0)
		{
			fprintf(fp_ret,"创建线程错误");
			exit(1);
		}
	}

	for(int i=0;i<num_test;i++)
		pthread_join(id[i],NULL);

	index_test=0;
*/
	for(int i=0;i<num_test;i++)
	{
		int ret=pthread_create(&(id[i]),NULL,test_GetRecByID,NULL);
		if(ret!=0)
		{
			fprintf(fp_ret,"创建线程错误");
			exit(1);
		}
	}

	for(int i=0;i<num_test;i++)
		pthread_join(id[i],NULL);

	index_test=0;


	for(int i=0;i<num_test;i++)
	{
		int ret=pthread_create(&(id[i]),NULL,test_UpdateRecByID,NULL);
		if(ret!=0)
		{
			fprintf(fp_ret,"创建线程错误");
			exit(1);
		}
	}

	for(int i=0;i<num_test;i++)
		pthread_join(id[i],NULL);

	index_test=0;

	for(int i=0;i<num_test;i++)
	{
		int ret=pthread_create(&(id[i]),NULL,test_GetRecsByAttr,NULL);
		if(ret!=0)
		{
			fprintf(fp_ret,"创建线程错误");
			exit(1);
		}
	}

	for(int i=0;i<num_test;i++)
		pthread_join(id[i],NULL);

	index_test=0;


	for(int i=0;i<num_op;i++)
		{
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				time_use_print[i][j][0]=time_use[i][j][0];
				time_use_print[i][j][1]=time_use[i][j][0];
				time_use_print[i][j][2]=time_use[i][j][0];
				for(int k=1;k<num_test;k++)
				{
					
					if(time_use_print[i][j][0]>time_use[i][j][k])
						time_use_print[i][j][0]=time_use[i][j][k];
					
					time_use_print[i][j][1]+=time_use[i][j][k];
					
					if(time_use_print[i][j][2]<time_use[i][j][k])
						time_use_print[i][j][2]=time_use[i][j][k];
				}
				
				time_use_print[i][j][1]=time_use_print[i][j][1]/num_test;
			}
		}

	//结果处理

	for(int i=0;i<num_op;i++)
		{
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				time_use_print1[i][j][0]=time_use1[i][j][0];
				time_use_print1[i][j][1]=time_use1[i][j][0];
				time_use_print1[i][j][2]=time_use1[i][j][0];
				for(int k=1;k<num_test;k++)
				{
					
					if(time_use_print1[i][j][0]>time_use1[i][j][k])
						time_use_print1[i][j][0]=time_use1[i][j][k];
					
					time_use_print1[i][j][1]+=time_use1[i][j][k];
					
					if(time_use_print1[i][j][2]<time_use1[i][j][k])
						time_use_print1[i][j][2]=time_use1[i][j][k];
				}
				
				time_use_print1[i][j][1]=time_use_print1[i][j][1]/num_test;
			}
		}


	
	fprintf(fp_ret,"\n-----------------------out function main------------------\n");
	
	fprintf(fp_time,"\n#### num= %d ,并发=%d，数据间隔=%d#########\n",num,num_test,interval_op);
	fprintf(fp_time,"\n**********************读 写操作时间**************\n");
	
	for(int i=0;i<num_op;i++)
		{
		fprintf(fp_time,"操作%d 最小时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print[i][j][0]);
			}
		fprintf(fp_time,"\n");
		
		fprintf(fp_time,"操作%d 平均时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print[i][j][1]);
			}
		fprintf(fp_time,"\n");
		
		fprintf(fp_time,"操作%d 最大时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print[i][j][2]);
			}
		fprintf(fp_time,"\n\n");
		}


		
	
fprintf(fp_time,"\n*****************更小的读 写操作时间**************\n");
	
	for(int i=0;i<num_op;i++)
		{
		fprintf(fp_time,"操作%d 最小时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print1[i][j][0]);
			}
		fprintf(fp_time,"\n");
		
		fprintf(fp_time,"操作%d 平均时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print1[i][j][1]);
				time_pingjun[i]+=time_use_print1[i][j][1];
			}
		fprintf(fp_time,"\n");
		
		fprintf(fp_time,"操作%d 最大时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print1[i][j][2]);
			}
		fprintf(fp_time,"\n\n");
		}
	
	fprintf(fp_time,"\n**********************插入操作时间**************\n");
	for(int i=0;i<test_insert;i++)
		fprintf(fp_time,"%lf,",time_insert[i]);


}


void test_all_single()
{
	test_UpdateRecByID(NULL);
	
	test_update_CtableOp();

	for(int i=0;i<num_op;i++)
		{
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				time_use_print[i][j][0]=time_use[i][j][0];
				time_use_print[i][j][1]=time_use[i][j][0];
				time_use_print[i][j][2]=time_use[i][j][0];
				for(int k=1;k<num_test;k++)
				{
					
					if(time_use_print[i][j][0]>time_use[i][j][k])
						time_use_print[i][j][0]=time_use[i][j][k];
					
					time_use_print[i][j][1]+=time_use[i][j][k];
					
					if(time_use_print[i][j][2]<time_use[i][j][k])
						time_use_print[i][j][2]=time_use[i][j][k];
				}
				
				time_use_print[i][j][1]=time_use_print[i][j][1]/num_test;
			}
		}

	//结果处理

	for(int i=0;i<num_op;i++)
		{
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				time_use_print1[i][j][0]=time_use1[i][j][0];
				time_use_print1[i][j][1]=time_use1[i][j][0];
				time_use_print1[i][j][2]=time_use1[i][j][0];
				for(int k=1;k<num_test;k++)
				{
					
					if(time_use_print1[i][j][0]>time_use1[i][j][k])
						time_use_print1[i][j][0]=time_use1[i][j][k];
					
					time_use_print1[i][j][1]+=time_use1[i][j][k];
					
					if(time_use_print1[i][j][2]<time_use1[i][j][k])
						time_use_print1[i][j][2]=time_use1[i][j][k];
				}
				
				time_use_print1[i][j][1]=time_use_print1[i][j][1]/num_test;
			}
		}


	
	fprintf(fp_ret,"\n-----------------------out function main------------------\n");
	
	fprintf(fp_time,"\n#### num= %d ,并发=%d，数据间隔=%d#########\n",num,num_test,interval_op);
	fprintf(fp_time,"\n**********************读 写操作时间**************\n");
	
	for(int i=0;i<num_op;i++)
		{
		fprintf(fp_time,"操作%d 最小时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print[i][j][0]);
			}
		fprintf(fp_time,"\n");
		
		fprintf(fp_time,"操作%d 平均时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print[i][j][1]);
			}
		fprintf(fp_time,"\n");
		
		fprintf(fp_time,"操作%d 最大时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print[i][j][2]);
			}
		fprintf(fp_time,"\n\n");
		}


		
	
fprintf(fp_time,"\n*****************更小的读 写操作时间**************\n");
	
	for(int i=0;i<num_op;i++)
		{
		fprintf(fp_time,"操作%d 最小时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print1[i][j][0]);
			}
		fprintf(fp_time,"\n");
		
		fprintf(fp_time,"操作%d 平均时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print1[i][j][1]);
				time_pingjun[i]+=time_use_print1[i][j][1];
			}
		fprintf(fp_time,"\n");
		
		fprintf(fp_time,"操作%d 最大时间:\n",i+1);
		for(int j=0;j<SUM_NUM/interval_op;j++)
			{
				fprintf(fp_time,"%lf,",time_use_print1[i][j][2]);
			}
		fprintf(fp_time,"\n\n");
		}
	
	fprintf(fp_time,"\n**********************插入操作时间**************\n");
	for(int i=0;i<test_insert;i++)
		fprintf(fp_time,"%lf,",time_insert[i]);


}

int main(int argc, char **argv)
{

	TB_DESCR tb_des;
	int ret_open,ret_close=0;
	int ret_code=0;

	fp_ret=fopen("rtdb_test_result.txt","w+");
		if (fp_ret==NULL)
		{
			printf("fopen() error\n");
			return 0;
		} 

	fp_time=fopen("rtdb_time_result.txt","w+");
		if (fp_ret==NULL)
		{
			printf("fopen() error\n");
			return 0;
		} 
	fprintf(fp_ret,"**********************in function main**************\n");

	int tableid[9]={417,410,415,411,436,406,407,408,435};

	for(int tab_no=0;tab_no<9;tab_no++)
	{
	table_id=431+tab_no%2;

	key_stru.key = KEY_VALUE;
	key_stru.field_id = 0;
	key_stru.table_no = table_id;
	

	CCommon::keyid_to_long(&key_stru, &kid);
	kid_test=kid;

	//OpenTableByID(char *host_name, int context_id ,int app_id, int table_id, TB_DESCR* tb_descr);
	ret_open = OpenTableByID(NULL, 1, 100000, table_id,&tb_des);

	if(ret_open < 0)
		fprintf(fp_ret,"Error when open %d,ret_val=%d\n",table_id,ret_open);

	for(num= 1; num <=SUM_NUM; ++num)
	{		
		insert_record(&tb_des);
		
		kid++;

		

		if(num%(interval_op)==0)
		{
			fprintf(fp_ret,"\n##################### num= %d ###################\n",num);
//test_all();				
		}	
		

	}

	
	cout<<endl<<"------------num="<<num<<"---------------"<<endl;

	//num=0;
	
//	for(int i=0;i<10;i++)
//	test_all();


	void* ptr ;
	for(int i=0;i<10;i++)
//	test_GetRecsByAttr ( ptr )

// test_all_single();
	
	ret_close = CloseTable(&tb_des);
	}
	fprintf(fp_time,"\n**********************平均操作时间**************\n");
	for(int i=0;i<num_op;i++)
		fprintf(fp_time,"%lf \n",time_pingjun[i]/10);
	
	if(ret_close < 0)
		fprintf(fp_ret,"Error when close %d,ret_val=%d\n",table_id,ret_close);



	fclose(fp_ret);
	fclose(fp_time);
		
	
	return 0;

}

