/* 
	Процессор для запуска удалённого лоадера и передачи stdin в stdout
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <config.h>

#include "../include/lss_misc.h"


int 
main(int argc, char **argv)
{
	char host_name[255];
	unsigned char	buffer[256];
	ssize_t			size;
	
	DEFINE_ENV_PROCESSOR;
	
	gethostname(host_name, 255);
	
	if (check_argv_processor(argc, argv)!=CODE_OK) exit(1);
	
	if (lss_time) 	fprintf(stdout, "%s %s %s\n", ldrID, lss_date, lss_time);
	else			fprintf(stdout, "%s %s\n", ldrID, lss_date);
	
	fflush(stdout);
	
	for(;;) {
	    size = read(STDIN_FILENO, buffer, 255);
	    if (size==0) break;
	    if (size==-1) {
			fprintf(stderr, "[%s][%d:%d] Error: error read operation (%s)\n", host_name, getpid(), getppid(), strerror(errno));
			exit(1);
		}
		if (write(STDOUT_FILENO, buffer, size)==-1) {
			fprintf(stderr, "[%s][%d:%d] Error: error write operation (%s)\n", host_name, getpid(), getppid(), strerror(errno));
			exit(1);
		}
	}
	exit(0);
}
