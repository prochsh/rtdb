#include"db_struct_scada.h"
#include"db_api/rtdb_api.h"
#include"db_api/odb_common.h"
#include"system.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<sched.h>
#define exit_on_error() \
{\
	perror("#error!");\
	exit(1);\
}


#define exit_on_pthderror(errorno) \
{\
	char errsz[128];\
	bzero(buff,128);\
	fprintf(stderr,"[error!%s:%d:%s] %s\n"\
			,__FILE__\
			,__LINE__\
			,__FUNCTION__\
			,strerror_r(errorno,errsz,128));\
	exit(1);\
}


#define RECV_PTHREAD_NUM 10
struct 
{
	pthread_t pthd_id[RECV_PTHREAD_NUM];
	int N;
}recv_thd_pool;
pthread_t update_id;
typedef struct msg_type{
	long keyid;
	int fieldid;
	char value[64];
	int size;
}MSG;
#define Q_SIZE (2001)
#define BUFF_SIZE (128)
struct
{
	MSG Q[Q_SIZE];
	int head;
	int tail;
	pthread_mutex_t mutex;
	pthread_cond_t notempty,notfull;
}msgQ;

void init_msgQ();
void enqueue_msgQ(MSG msg);
MSG  dequeue_msgQ();
void destroy_msgQ();
int data_recv(int fd);
void intrhandler(int signum);
void cleanhandler();
int rtdb_update();
int insRec_transformerwinding(TB_DESCR*tb_des,MSG msg);
int insRec_busbarsection(TB_DESCR*tb_des,MSG msg);
int insRec_aclineend(TB_DESCR*tb_des,MSG msg);
int insRec_generatingunit(TB_DESCR*tb_des,MSG msg);
int insRec_measure(TB_DESCR*tb_des,MSG msg);
int insRec_bay(TB_DESCR*tb_des,MSG msg);
int insRec_breaker(TB_DESCR*tb_des,MSG msg);
int insRec_disconnector(TB_DESCR*tb_des,MSG msg);
int insRec_signal(TB_DESCR*tb_des,MSG msg);
int main()
{
	int listen_fd
		,accept_fd
		,pthd_errno
		,i
		,j;

	struct sockaddr_in servaddr,clientaddr;
	socklen_t clientaddr_len;
	char buff[BUFF_SIZE];
	struct sigaction  act;
	
	if ( (listen_fd=socket(AF_INET,SOCK_STREAM,0)) <0 )
		exit_on_error();
	
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family			= AF_INET;
	inet_pton(AF_INET,getenv("SERV_TST_IP"),&servaddr.sin_addr.s_addr);
	servaddr.sin_port		 	= htons(atoi(getenv("SERV_TST_PORT")));

	if ( bind(listen_fd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
		exit_on_error();
	if ( listen(listen_fd,RECV_PTHREAD_NUM) < 0 )
		exit_on_error();
	
	bzero(buff,BUFF_SIZE);
	printf( "%s is listen at port %d\n"
		   , inet_ntop(AF_INET,&servaddr.sin_addr,buff,INET_ADDRSTRLEN)
		   , ntohs(servaddr.sin_port));

	init_msgQ();

	if (pthd_errno=pthread_create(&update_id,NULL,(void*(*)(void*))&rtdb_update,NULL))
		exit_on_pthderror(pthd_errno);

	act.sa_handler = intrhandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	sigaction(SIGINT,&act,NULL);
	atexit(&cleanhandler);
	
	bzero((char*)&recv_thd_pool,sizeof(recv_thd_pool));
	while ( recv_thd_pool.N < RECV_PTHREAD_NUM ) {
		
		clientaddr_len = sizeof (clientaddr);
		accept_fd = accept( listen_fd
						  , (struct sockaddr*)&clientaddr
						  , &clientaddr_len );

		bzero(buff,BUFF_SIZE);
		printf( "[%d]%s:%d\n"
			  , recv_thd_pool.N
			  , inet_ntop(AF_INET,&clientaddr.sin_addr,buff,INET_ADDRSTRLEN)
			  , ntohs(clientaddr.sin_port));

		pthd_errno=pthread_create(&(recv_thd_pool.pthd_id[recv_thd_pool.N++])
								 ,NULL
								 ,(void*(*)(void*))&data_recv
								 ,(void*)accept_fd);

		if (pthd_errno)
			exit_on_pthderror(pthd_errno);
	}

	pthread_join(update_id,NULL);
	for (i=0;i<recv_thd_pool.N;i++) {
		pthread_join( recv_thd_pool.pthd_id[i],NULL);
	}
	return 0;
}

void init_msgQ()
{
	msgQ.head=-1;
	msgQ.tail=0;
	bzero(msgQ.Q,sizeof(msgQ.Q));
	pthread_mutex_init(&msgQ.mutex,NULL);
	pthread_cond_init(&msgQ.notempty,NULL);
	pthread_cond_init(&msgQ.notfull,NULL);
}
void destroy_msgQ()
{
	pthread_mutex_destroy(&msgQ.mutex);
	pthread_cond_destroy(&msgQ.notempty);
	pthread_cond_destroy(&msgQ.notfull);
}

void enqueue_msgQ(MSG msg)
{
	pthread_mutex_lock(&msgQ.mutex);
	while ( msgQ.tail == msgQ.head  )
		pthread_cond_wait(&msgQ.notfull,&msgQ.mutex);

	memcpy(&(msgQ.Q[msgQ.tail]),&msg,sizeof(msg));
	msgQ.tail=(msgQ.tail+1)%Q_SIZE;

	pthread_mutex_unlock(&msgQ.mutex);
	pthread_cond_broadcast(&msgQ.notempty);
	printf("enqueue %s\n",msg.value);

}
MSG dequeue_msgQ()
{
	MSG msg;	
	pthread_mutex_lock(&msgQ.mutex);
	while ( (msgQ.head+1)%Q_SIZE==msgQ.tail )
		pthread_cond_wait(&msgQ.notempty,&msgQ.mutex);

	msgQ.head = (msgQ.head+1)%Q_SIZE;
	memcpy(&msg,&(msgQ.Q[msgQ.head]),sizeof(msg));
	pthread_mutex_unlock(&msgQ.mutex);
	pthread_cond_broadcast(&msgQ.notfull);
	return msg;
	printf("dequeue %ld...",msg.value);
}


int data_recv(int fd)
{

	printf("enter thread whose socket's fd =%d\n",fd);
	MSG msg;
	int size;
	while(1){
		if ( read(fd,&msg,sizeof(MSG))== sizeof(MSG) ){
			enqueue_msgQ(msg);
			printf("%ld:%s\n",msg.keyid,msg.value);
		}
	}
	return  0;
}
int rtdb_update()
{
	printf("enter rtdb_update thread\n");
	MSG msg;
	int round,count,ret_code;
	round=0;
	TB_DESCR tb_des;
	KEY_STRU key_stru;
	while (1){
		
		for ( count=0;count<20;count++ ) {
			printf("dequeuing...\n");
			msg = dequeue_msgQ();
			printf("...dequeued\n");
			CCommon::long_to_keyid(msg.keyid,&key_stru);

			if (OpenTableByID(NULL,AC_REALTIME_NO,AP_SCADA,key_stru.table_no,&tb_des) <0){
				fprintf( stderr
					   , "[error!%s:%d:%s]can not open table %d\n"
					   , __FILE__
					   , __LINE__
					   , __FUNCTION__
					   , key_stru.table_no );
				continue;
			}
			printf("succeed in opening table %d!\n",key_stru.table_no);
			if (UpdateFieldsByID( &tb_des
						        , (char *)&msg.keyid
						        , &(msg.fieldid)
								, 1
								, msg.size
								, msg.value)<0) {
				printf( "fail in updating table %d record %d\n"
					  , key_stru.table_no
					  , key_stru.key);

				switch (key_stru.table_no){
				case 417://transformerwinding
					ret_code=insRec_transformerwinding(&tb_des,msg);
					break;
				case 410://busbarsection
					ret_code=insRec_busbarsection(&tb_des,msg);
					break;
				case 415://aclineend
					ret_code=insRec_aclineend(&tb_des,msg);
					break;
				case 411://generatingunit
					ret_code=insRec_generatingunit(&tb_des,msg);
					break;
				case 436://measure
					ret_code=insRec_measure(&tb_des,msg);
					break;
				case 406://bay
					ret_code=insRec_bay(&tb_des,msg);
				case 407://breaker
					ret_code=insRec_breaker(&tb_des,msg);
				case 408://disconnector
					ret_code=insRec_disconnector(&tb_des,msg);
					break;
				case 435://signal
					ret_code=insRec_signal(&tb_des,msg);
					break;
				default :
					fprintf( stderr
						   , "[error!%s:%d:%s]invalid table_no %d\n"
						   , __FILE__
						   , __LINE__
						   , __FUNCTION__
						   , key_stru.table_no );
					ret_code=-1;
					break;
				}

				if (ret_code<0){
					fprintf( stderr
						   , "[error!%s:%d:%s]fail in inserting record %d into table_no %d\n"
						   , __FILE__
						   , __LINE__
						   , __FUNCTION__
						   , key_stru.key
						   , key_stru.table_no );
					continue;
				}
				printf("succeed in inserting record%s\n",msg.value);
			}
			printf("succeed in inserting record%s\n",msg.value);
			CloseTable(&tb_des);
		}
		round++;
	}
	return 0;
}

int insRec_transformerwinding(TB_DESCR*tb_des,MSG msg)
{
	TRANSFORMERWINDING_SCADA  rec;
	rec.id=msg.keyid;
	memcpy(&(rec.name),&(msg.value),msg.size);
	return (tb_des,1,sizeof(TRANSFORMERWINDING_SCADA),(char*)&rec,1);
}
int insRec_busbarsection(TB_DESCR*tb_des,MSG msg){
	BUSBARSECTION_SCADA  rec;
	rec.id=msg.keyid;
	memcpy(&(rec.name),&(msg.value),msg.size);
	return InsertRec(tb_des,1,sizeof(BUSBARSECTION_SCADA),(char*)&rec,1);
}
int insRec_aclineend(TB_DESCR*tb_des,MSG msg){
	ACLINEEND_SCADA rec;
	rec.id=msg.keyid;
	memcpy(&(rec.name),&(msg.value),msg.size);
	return InsertRec(tb_des,1,sizeof(ACLINEEND_SCADA),(char*)&rec,1);
}
int insRec_generatingunit(TB_DESCR*tb_des,MSG msg){
	GENERATINGUNIT_SCADA rec;
	rec.id=msg.keyid;
	memcpy(&(rec.name),&(msg.value),msg.size);
	return InsertRec(tb_des,1,sizeof(GENERATINGUNIT_SCADA),(char*)&rec,1);
}
int insRec_measure(TB_DESCR*tb_des,MSG msg){
	MEASURE_SCADA rec;
	rec.id=msg.keyid;
	memcpy(&(rec.name),&(msg.value),msg.size);
	return InsertRec(tb_des,1,sizeof(MEASURE_SCADA),(char*)&rec,1);
}
int insRec_bay(TB_DESCR*tb_des,MSG msg){
	BAY_SCADA rec;
	rec.id=msg.keyid;
	memcpy(&(rec.name),&(msg.value),msg.size);
	return InsertRec(tb_des,1,sizeof(BAY_SCADA),(char*)&rec,1);
}
int insRec_breaker(TB_DESCR*tb_des,MSG msg){
	BREAKER_SCADA rec;
	rec.id=msg.keyid;
	memcpy(&(rec.name),&(msg.value),msg.size);
	return InsertRec(tb_des,1,sizeof(BREAKER_SCADA),(char*)&rec,1);
}
int insRec_disconnector(TB_DESCR*tb_des,MSG msg){
	DISCONNECTOR_SCADA rec;
	rec.id=msg.keyid;
	memcpy(&(rec.name),&(msg.value),msg.size);
	return InsertRec(tb_des,1,sizeof(DISCONNECTOR_SCADA),(char*)&rec,1);
}
int insRec_signal(TB_DESCR*tb_des,MSG msg){
	SIGNAL_SCADA rec;
	rec.id=msg.keyid;
	memcpy(&(rec.name),&(msg.value),msg.size);
	return InsertRec(tb_des,1,sizeof(SIGNAL_SCADA),(char*)&rec,1);
}
void cleanhandler()
{

	fprintf(stderr,"do some cleanning before exit\n ");
	destroy_msgQ();
}
void intrhandler(int signum)
{
	fprintf(stderr,"exit resulting from interrupt!\n");
	exit(1);
}
