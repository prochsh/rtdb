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
#include "sys/time.h"
#include <time.h>
#include <sys/timeb.h>

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
#define 	  AP_SCADA				 100000

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

	//if (pthd_errno=pthread_create(&update_id,NULL,(void*(*)(void*))&rtdb_update,NULL))
	//	exit_on_pthderror(pthd_errno);

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
	/*	
		printf( "[%d]%s:%d\n"
			  , recv_thd_pool.N
			  , inet_ntop(AF_INET,&clientaddr.sin_addr,buff,INET_ADDRSTRLEN)
			  , ntohs(clientaddr.sin_port));
*/
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
	//printf("enqueue %s\n",msg.value);

}
MSG dequeue_msgQ()//解析一个点
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
	//printf("dequeue %ld...",msg.value);
}


int data_recv(int fd)
{

	printf("enter thread whose socket's fd =%d\n",fd);
	MSG msg;
	int size;
	int count,ret_code_open,ret_code_update,ret_code_read;
	TB_DESCR tb_des;
	KEY_STRU key_stru;
	CTableOp m_tableop;

	float timeuse=0;
	float time_max=0;
	float time_avrage=0;
	double time_sum=0;
	long sum=0;
	struct timeval tpstart,tpend;

	float timeuse1=0;
	float time_max1=0;
	float time_avrage1=0;
	double time_sum1=0;
	long sum1=0;


	float timeuse2=0;
	float time_max2=0;
	float time_avrage2=0;
	double time_sum2=0;
	long sum2=0;

	while(1)
		{
		
	gettimeofday(&tpstart,NULL);
		
		size=read(fd,&msg,sizeof(MSG));

	gettimeofday(&tpend,NULL);

			sum1++;
	
			timeuse1=1000000*(tpend.tv_sec-tpstart.tv_sec)+
                	tpend.tv_usec-tpstart.tv_usec;
                	timeuse1/=1000;//毫秒

			if(time_max1<timeuse1)
				time_max1=timeuse1;
			time_sum1+=timeuse1;
			time_avrage1=time_sum1/sum1;

			cout<<"max1="<<time_max1<<",avr1="<<time_avrage1<<",sum1="<<sum1<<",time_sum1="<<time_sum1<<endl;
		
		if ( size== sizeof(MSG) )
		{

			
				msg.fieldid=3;
				cout<<msg.keyid<<","<<msg.value<<","<<msg.size<<endl;
				CCommon::long_to_keyid(msg.keyid,&key_stru);


	gettimeofday(&tpstart,NULL);

				ret_code_open = m_tableop.Open(AP_SCADA, key_stru.table_no);
	gettimeofday(&tpend,NULL);

	
			timeuse2=1000000*(tpend.tv_sec-tpstart.tv_sec)+
                	tpend.tv_usec-tpstart.tv_usec;
                	timeuse2/=1000;//毫秒

			sum2++;

			if(time_max2<timeuse2)
				time_max2=timeuse2;
			time_sum2+=timeuse2;
			time_avrage2=time_sum2/sum2;

			cout<<"max2="<<time_max2<<",avr2="<<time_avrage2<<",sum2="<<sum2<<",time_sum2="<<time_sum2<<endl;
	
	gettimeofday(&tpstart,NULL);
    				
				ret_code_update=m_tableop.TableModifyByKey((char*)&(msg.keyid),msg.fieldid,msg.value,msg.size);

	gettimeofday(&tpend,NULL);

				if (ret_code_open != DB_OK)
        				printf("打开错误 %d\n",key_stru.table_no);
				
				if (ret_code_update != DB_OK)
					printf("更新表错误 %ld %d\n",msg.keyid,ret_code_update);
				

			sum++;
	
			timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+
                	tpend.tv_usec-tpstart.tv_usec;
                	timeuse/=1000;//毫秒

			if(time_max<timeuse)
				time_max=timeuse;
			time_sum+=timeuse;
			time_avrage=time_sum/sum;

			cout<<"max="<<time_max<<",avr="<<time_avrage<<",sum="<<sum<<",time_sum="<<time_sum<<endl;
                	
		}
		
	}
	return  0;
}
int rtdb_update()
{
	//printf("enter rtdb_update thread\n");
	MSG msg;
	int round,count,ret_code;
	round=0;
	TB_DESCR tb_des;
	KEY_STRU key_stru;
	CTableOp m_tableop;

	float timeuse=0;
	float time_max=0;
	float time_avrage=0;
	double time_sum=0;
	int sum=0;

	//int table[9]={417,410,415,411,436,406,407,408,435};

	while (1)
		{
			struct timeval tpstart,tpend;
			gettimeofday(&tpstart,NULL);
			{
				
				msg = dequeue_msgQ();
				msg.fieldid=3;
				cout<<msg.keyid<<","<<msg.fieldid<<","<<msg.value<<","<<msg.size<<endl;
				CCommon::long_to_keyid(msg.keyid,&key_stru);

				ret_code = m_tableop.Open(AP_SCADA, key_stru.table_no);
    				if (ret_code != DB_OK)
   				{
        			printf("QQQQQQQQQQQ---table open error %d\n",key_stru.table_no);
        			//return  -1;
    				}

				//CTableOp::TableModifyByKey(const char* key_ptr, const int field_no, const char* field_buf_ptr, const int buf_size, bool is_map)
				m_tableop.TableModifyByKey((char*)&(msg.keyid),msg.fieldid,msg.value,msg.size);

				gettimeofday(&tpend,NULL);
				
				if (ret_code != DB_OK)
				{
				printf("QQQQQQQQQQ---table modify error %ld %d\n",msg.keyid,ret_code);
				//return	-1;
				}

			}
			sum++;
			if(sum>1)
			{
			timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+
                	tpend.tv_usec-tpstart.tv_usec;
                	timeuse/=1000;//毫秒

			if(time_max<timeuse)
				time_max=timeuse;
			time_sum+=timeuse;
			time_avrage=time_sum/sum;

			cout<<"max="<<time_max<<","<<"avr="<<time_avrage<<","<<"sum="<<sum<<endl;
			}
                	
		}
	return 0;
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
