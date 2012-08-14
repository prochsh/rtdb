#include "sys/time.h"
#include "rtdb_api.h"
#include "db_api/odb_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <pthread.h>


	struct testdata
	{
		unsigned long id;
		char ename[32];
		char cname[64];
		long st_id;//no use
		long alg_id;//no use
		float ori_value;
		int flags;
		float valid_up;
		float valid_low;
		
		char other[520-136];
	};

struct id_code_name_stru
{
	long id;
	char code[32];
	char name[64];
	long st_id;//no use
	long alg_id;//no use
	float ori_value;
	int flags;
	float valid_up;
	float valid_low;
};

