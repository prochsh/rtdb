#include"system.h"
#include"db_api/odb_struct.h"
#include"db_api/odb_common.h"
#include"db_api/rtdb_api.h"

#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define exit_on_error() \
{\
	perror("#error!");\
}
typedef struct msg_type{
	unsigned long keyid;
	int fieldid;
	char value[64];
	int size;
}MSG;

int main(int argc, char **argv){
	int connect_fd;
	struct sockaddr_in servaddr;
	int table[2][5]={
		             {417,410,415,411,436},
		             {406,407,408,435,  0}
	                };
	int k,tabnum,msgcnt,pkgcnt;
	long i;
	MSG msgpkg[100];
	KEY_STRU key_stru;
	if (argc!=2){
		fprintf(stderr,"usage: prog <[0|1]>\n");
		fprintf(stderr,"       0:sca_analog\n");
		fprintf(stderr,"       1:sca_point\n");

		exit(1);
	}
	
	if (!strcmp(argv[1],"0")) {
		k=0;
		tabnum=5;
	}else if (!strcmp(argv[1],"1")) {
		k=1;
		tabnum=4;
	}else {
		fprintf(stderr,"#error!%s-%d:[%s]\n",__FILE__,__LINE__,__FUNCTION__);
		exit(1);
	}

	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET,getenv("SERV_TST_IP"),&servaddr.sin_addr);
	servaddr.sin_port   = htons(atoi(getenv("SERV_TST_PORT")));

	if ( ( connect_fd=socket(AF_INET,SOCK_STREAM,0) ) < 0 )
		exit_on_error();
	if ( connect(connect_fd,(struct sockaddr*)&servaddr,sizeof(servaddr)) )
		exit_on_error();

	fprintf(stderr,"#error!%s-%d:[%s]\n",__FILE__,__LINE__,__FUNCTION__);
	i=0;
	while (i<tabnum*0x7fffffffL) {

		for ( pkgcnt=0;pkgcnt<10;pkgcnt++ ){
			for ( msgcnt=0;msgcnt<100;msgcnt++) {
				key_stru.key		= i/tabnum;
				key_stru.field_id	= 0;
				key_stru.table_no	= table[k][i%tabnum];	
				CCommon::keyid_to_long(&key_stru,&msgpkg[msgcnt].keyid);
				msgpkg[msgcnt].fieldid=0;
				sprintf( msgpkg[msgcnt].value
					   , "±íºÅ:%d,¼ÇÂ¼:%d"
					   , key_stru.table_no
					   , key_stru.key);
				printf("table_no:%ld,record:%s\n",msgpkg[msgcnt].keyid,msgpkg[msgcnt].value);
				msgpkg[msgcnt].size=64;
				i++;
			}
			fprintf(stderr,"#error!%s-%d:[%s]\n",__FILE__,__LINE__,__FUNCTION__);
			write(connect_fd,msgpkg,sizeof(msgpkg));
		}
		sleep(1);
	}
	return 0;
}
