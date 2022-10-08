/* Created by Anjuta version 1.2.2 */
/*	This file will not be overwritten */
/*
	SQL_PROC
	
	/configuration/loader[@name='%s']/processor[@name='%s']/param[@name='options']" -> value
	

*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/wait.h>

#include <config.h>

#include "../include/lss_misc.h"
#include "../include/lss_conf.h"

#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>



char 	*NAME_PROCESSOR;
char 	host_name[255];
pid_t	pid = 0;
char	tmp_name[FILENAME_MAX], file_stdin[FILENAME_MAX], file_stderr[FILENAME_MAX];

RETSIGTYPE sig_term(int signo) {
	if (pid>0) kill(pid, SIGTERM);
	exit(1);
}
//--------------------------------
// Prototype Private functions	 |
//--------------------------------	
static int 		load_param(const char *config_file, const char *ldrID, const char *date, const char *time, char *arg_v[]);
static int		fifo_create(char *file);
static void		sig_kill(int sig);

//-----------------------------------------------------------------------------
//									BEGIN PROGRAM
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    char 	*arg_v[30];
	int	fifo_in;
	
	DEFINE_ENV_PROCESSOR;
	
	NAME_PROCESSOR = strdup(basename(argv[0]));
	gethostname(host_name, 255);
	
	if (check_argv_processor(argc, argv)!=CODE_OK) exit(1);
	
	arg_v[0] = strdup(NAME_PROCESSOR);
	
	tmpnam(tmp_name);
	
	sprintf(file_stdin, "%s.stdin", tmp_name);
	sprintf(file_stderr, "%s.log", tmp_name);
	
	if (fifo_create(file_stdin)!=CODE_OK) exit(1);

	signal(SIGTERM, sig_term);
	signal(SIGQUIT, sig_term);
	signal(SIGINT,	sig_term);
	
	pid = fork();
	
	if (pid == (pid_t) -1) {
		CRIT_PROC_ERROR(NAME_PROCESSOR, "fork", 1);
	} else if (pid == 0) {
		char			tmp_string[200];
		XML_SESSION		*sid;
		
		sprintf(tmp_string, "data=%s", file_stdin);
		arg_v[1] = strdup(tmp_string);
		sprintf(tmp_string, "log=%s", file_stderr);
		arg_v[2] = strdup(tmp_string);
		
		if ((sid =lss_xml_init(config_file))==NULL){
			unlink(file_stdin);
			unlink(file_stderr);
			exit(1);			
		}
		
		if (lss_xml_get_options(sid, ldrID, procID, 3, arg_v, 30)!=CODE_OK) {
			unlink(file_stdin);
			unlink(file_stderr);
			exit(1);
		}
		lss_xml_free(sid);
		
		fclose(stdout);		
		
		execvp("sqlldr", arg_v);
		CRIT_PROC_ERROR(NAME_PROCESSOR, "execvp", 1);		
	}else{					// PARENT
		int				status;
		pid_t			t_pid;
		unsigned char	buffer[256];
		ssize_t			size;
		FILE			*err_fd;

		signal(SIGCHLD, sig_kill);
		
		if ((fifo_in = open(file_stdin, O_WRONLY))==-1) 
		{
		    fprintf(stderr, "[%s][%d:%d] Error: error open fifo '%s'\n", host_name, getpid(), getppid(), file_stdin);
		    unlink(file_stdin);
		    unlink(file_stderr);
		    exit(1);
		}
		
		for(;;) {
		    size = read(STDIN_FILENO, buffer, 255);
		    if (size==0) break;
		    if (size==-1) {
			fprintf(stderr, "[%s][%d:%d] Error: error read operation (%s)\n", host_name, getpid(), getppid(), strerror(errno));
			exit(1);
		    }
		    if (write(fifo_in, buffer, size)==-1) {
			fprintf(stderr, "[%s][%d:%d] Error: error write operation\n", host_name, getpid(), getppid());
			exit(1);
		    }
		}
		close(fifo_in);

		signal(SIGCHLD, SIG_DFL);
		
		for(;;){
			t_pid = waitpid(pid, &status, 0);
			if( t_pid == -1 && errno == EINTR ) continue;
			if (t_pid == -1) {
				unlink(file_stdin);
				unlink(file_stderr);
				CRIT_PROC_ERROR(NAME_PROCESSOR, "waitpid", 1)
			}
			if (t_pid > 0) {
			    if ((err_fd = fopen(file_stderr, "r"))==NULL){
					fprintf(stderr, "[%s][%d:%d] Error: error open fifo '%s'\n", host_name, getpid(), getppid(), file_stderr);
					unlink(file_stdin);
					unlink(file_stderr);
					exit(1);
		    	}
		
		    	while(!feof(err_fd)){
					if (fgets(buffer, 255, err_fd)!=NULL) fputs(buffer, stderr);
		    	}
		    	fclose(err_fd);
				    
			    unlink(file_stdin);
		    	unlink(file_stderr);				
			    
			    if (status==0)	fprintf(stdout, "ok\n");
		    	else			fprintf(stdout, "error\n");
			    fflush(stdout);
		    	exit(status!=0);
			}
		 }
	}
}

//-----------------------------------------------------------------------------
//                              fifo_create
//-----------------------------------------------------------------------------
static int
fifo_create(char *file)
{
	assert(file);
 
	umask(0);
                                                                                                                                                             
 	if (mkfifo(file, S_IRWXU) == -1) {
  		fprintf(stderr, "[%s][%d:%d] Error: error create fifo %s (%s)\n", host_name, getpid(), getppid(), file, strerror(errno));
  		return CODE_ERR;
 	}
 	return CODE_OK;
}

//-----------------------------------------------------------------------------
//                              sig_kill
//-----------------------------------------------------------------------------
static void		
sig_kill(int sig)
{
	pid_t	t_pid;
	int		status = 0;
	
	t_pid = waitpid(pid, &status, 0);

	for(;;) {
	    pid = waitpid( -1, &status, WNOHANG );
	    if( pid == -1 && errno == EINTR )  continue;
	    if( pid <= 0 )  break;
		if (status) {
		    unlink(file_stdin);
		    unlink(file_stderr);
		    exit (status);
		}
	}
}
