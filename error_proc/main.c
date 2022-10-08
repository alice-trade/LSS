/* 
	Процессор заглушка для выдачи ошибочного результата
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
	fprintf(stdout, "error\n");
	exit(1);
}
