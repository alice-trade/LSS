/* 
 	Процессор для запуска удалённого лоадера
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#include <config.h>
#include "../include/lss_misc.h"


int 
main(int argc, char **argv)
{
	DEFINE_ENV_PROCESSOR;
	
	if (check_argv_processor(argc, argv)!=CODE_OK) exit(1);
	
	if (lss_time) 	fprintf(stdout, "%s %s %s\n", ldrID, lss_date, lss_time);
	else			fprintf(stdout, "%s %s\n", ldrID, lss_date);

	fflush(stdout);
	
	exit(0);
}
