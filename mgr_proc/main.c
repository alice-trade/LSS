/*
	Функция процессора - запуск менеджеров
*/

#include <stdio.h>
#include <sys/types.h>  /* include this before any other sys headers */
#include <sys/wait.h>   /* header for waitpid() and various macros */
#include <signal.h>     /* header for signal functions */
#include <stdio.h>      /* header for fprintf() */
#include <unistd.h>     /* header for fork() */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>	   
	  
#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>

#include "../include/lss_misc.h"


char host_name[255];

int 
start_manager(const char *name, const char *t_date, const char *t_time) 
{
    pid_t 	p;
	char	loader_path[FILENAME_MAX];

    p = fork();
    
    if (p == (pid_t) -1) {
		CRIT_MANAGER_ERROR("mgr_proc", "fork", 1);		/* ERROR */
    } else if (p == 0) {    	/* CHILD */
		char host_name[255];
		
		gethostname(host_name, 255);
		sprintf(loader_path, "%s/%s", LOADER_DIR, "lss_mgr");
		if (!t_date) {
			execl(loader_path, "lss_mgr", name, NULL);
			CRIT_MANAGER_ERROR("lss_mgr", "execl", 1);
		}else if (!t_time) {
			execl(loader_path, "lss_mgr", name, t_date, NULL);
			CRIT_MANAGER_ERROR("lss_mgr", "execl", 1);
		} else {
			execl(loader_path, "lss_mgr", name, t_date, t_time, NULL);
			CRIT_MANAGER_ERROR("lss_mgr", "execl", 1);
		}
    }else	return p;
}

int 
main(int argc, char *argv[]) 
{
    int 	pid, w_pid, status = 0;
	char	buffer[50];
	ssize_t	size;
	
	DEFINE_ENV_PROCESSOR;
	bzero(buffer, 50);
	
	gethostname(host_name, 255);
	
	if (check_argv_processor(argc, argv)!=CODE_OK) exit(1);
	
	size = read(STDIN_FILENO, buffer, 49);
	if (size==-1) {
		fprintf(stderr, "[%s][%d:%d] Error: error read operation (%s)\n", host_name, getpid(), getppid(), strerror(errno));
		exit(1);
	}

	if (strncmp(buffer, "ok", 2)!=0) {
		fprintf(stdout, "error\n");
		fflush(stdout);
		fprintf(stderr, "[%s][%d:%d] Error: Last processor exit with error code\n" , host_name, getpid(), getppid());
		exit(1);
	}
		
	if ((pid = start_manager(procID, lss_date, lss_time))==0) exit(1);
	
	status = 0;
	for(;;) {
		w_pid = waitpid(pid, &status, 0);
		if (w_pid > 0) break;
		if( w_pid == -1 && errno == EINTR )  continue;
		if( pid < 0)  break;
    }
	if (status==0) 	fprintf(stdout, "ok\n");
	else{
		fprintf(stdout, "error\n");
		fprintf(stderr, "[%s][%d:%d] Error: This processor exit with error code\n" , host_name, getpid(), getppid());
	}
	
	fflush(stdout);
	fprintf(stderr, "status=%d\n", status);
	exit(status!=0);
}
