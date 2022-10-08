
#include <config.h>

#include <stdio.h>

#if HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#include <assert.h>


#include <signal.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <termios.h>

#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>

#include "../include/lss_misc.h"
#include "../include/lss_conf.h"


pid_t 	pgid = -1;
int		timeout = 10;
int		retry = 3;
int		retry_wait = 10;
int		child_terminate = 0;
char 	host_name[255];	

char module_path[FILENAME_MAX];

char modules_dir[FILENAME_MAX];

// NOT FOR MULTI THREADED APPLICATIONS
char *get_module_path(const char *name) {
    snprintf(module_path,FILENAME_MAX,"%s/%s",modules_dir,name);
    return(module_path);
}

RETSIGTYPE sig_term(int signo) {
	if(pgid > 0) kill(-pgid, SIGTERM);
	exit(1);
}

RETSIGTYPE sig_alarm(int signo) {
	if( signo != SIGALRM ) return;
	if(pgid > 0) kill(-pgid, SIGTERM);
}

RETSIGTYPE childs_terminate(int signo) {
	child_terminate = 1;
	if(pgid > 0) kill(-pgid, SIGTERM);
}

RETSIGTYPE terminate(int signo) {
	childs_terminate(signo);
	exit(1);
}

//--------------------------------
// Prototype Public functions	 |
//--------------------------------

static int load_processors(XML_SESSION *sid, const char *ldrID, char *date, char *time);
static int get_param(XML_SESSION *sid, const char *ldrID, int *timeout, int *retry, int *retry_wait);

//--------------------------------
// Prototype Private functions   |
//--------------------------------
int start_processor(const char *config_file, const char *ldrID, const char *ProcID, const char *name, 
    const char *date, const char *time, int stream_in, int stream_out);



//-----------------------------------------------------------------------------
//									BEGIN PROGRAM
//-----------------------------------------------------------------------------
int 
main(int argc, char **argv)
{
	int				pid, status, result;
	struct			termios settings;
	XML_SESSION		*sid;
	
	DEFINE_ENV_LOADER;			// bzero variable
	
	if ( tcgetattr (STDOUT_FILENO, &settings) >= 0 ) {
            settings.c_lflag &= ~TOSTOP;
	    tcsetattr(STDOUT_FILENO,TCSANOW,&settings);
	}

	signal(SIGTERM, sig_term);
	signal(SIGQUIT, sig_term);
	signal(SIGINT,	sig_term);
	signal(SIGALRM, sig_alarm); 
	
	gethostname(host_name, 255);	

	if (check_argv_loader(argc, argv)!=CODE_OK) exit(1);

	if ((sid = lss_xml_init(CONFIG_FILE))==NULL) exit(1);	

	if (lss_xml_get_value(sid, "/configuration/directory/processors[@value]", modules_dir, sizeof(modules_dir))!=CODE_OK) exit(1);

	if (get_param(sid, ldrID, &timeout, &retry, &retry_wait)!=CODE_OK) exit(1);	
	
	fprintf(stderr,"[%s][%d] Loader Start %s %s %s\n", host_name, getpid(), ldrID, lss_date, lss_time);
	
	for(;;){
		alarm(timeout);
		if (load_processors(sid, ldrID, lss_date, lss_time)!=CODE_OK) 
		    terminate(SIGTERM);
		
		result = 0;
		for(;;) {
	    	pid = waitpid( -1, &status, 0 );

	    	if( pid == -1 && errno == EINTR )  continue;

			if(status != 0 && !child_terminate){ 
	    		childs_terminate(SIGTERM);
				fprintf(stderr, "SEND TERM SIGNAL\n");
			}
	    	if( pid < 0 )  break;
	    	fprintf(stderr,"[%s][%d] Processor %d exit with %d code result\n", host_name, getpid(), pid, status);
			result = result || (status != 0);
		}
		
		if (!result || !--retry) break;
		sleep(retry_wait);
		
		pgid = -1;
		child_terminate = 0;

		fprintf(stderr, "[%s][%d] Loader '%s' Retry exit with %d code result\n\n", host_name, getpid(), ldrID, (result != 0));
	}
	fprintf(stderr, "[%s][%d] Loader %s exit with %d code result\n", host_name, getpid(), ldrID, (result != 0));
	
	lss_xml_free(sid);
	exit(result != 0);
}



//-----------------------------------------------------------------------------
// load_processors
//-----------------------------------------------------------------------------
static int 
load_processors(XML_SESSION *sid, const char *ldrID, char *date, char *time)
{
    char 				xpath_query[255];
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size, i;
    int 				stream[2], start_in, stop_out;

	sprintf(xpath_query, "/configuration/loader[@name='%s']", ldrID);

	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
	if(xpathObj == NULL) {
		fprintf(stderr,"[%s][%d:%d] Loader Error: unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
        return CODE_ERR;
    }
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	xmlXPathFreeObject(xpathObj);
		
	if (size>0) {
		sprintf(xpath_query, "/configuration/loader[@name='%s']/processor", ldrID);

		xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
		if(xpathObj == NULL) {
			fprintf(stderr,"[%s][%d:%d] Loader Error: unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
        	return CODE_ERR;
    	}


    	nodes = xpathObj->nodesetval;
		size = (nodes) ? nodes->nodeNr : 0;
    
    	start_in = STDIN_FILENO;
    	for(i = 0; i < size; ++i) {
			assert(nodes->nodeTab[i]);
			if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {	// element node?
				if ( i < (size - 1) ) {
					if ( pipe(stream) ) CRIT_LOADER_ERROR(ldrID, "pipe_stream", 1);	/* ERROR */
					stop_out = stream[1];
	    		} else {
					stop_out = STDOUT_FILENO;
	    		}
	    		if (start_processor(sid->config_file, ldrID, xmlGetProp(nodes->nodeTab[i], "procID"), xmlGetProp(nodes->nodeTab[i], "name"), date, time, start_in, stop_out) <= 0) {	// start_processors
					fprintf(stderr,"[%s][%d:%d] Loader Error: error start processor '%s'\n", host_name, getpid(), getppid(), xmlGetProp(nodes->nodeTab[i], "name"));
					
					return CODE_ERR;
	    		}
	    		start_in = stream[0];
			}
    	}

		xmlXPathFreeObject(xpathObj);
	}else {
		fprintf(stderr,"[%s][%d:%d] Loader Error: loader '%s' not found\n", host_name, getpid(), getppid(), ldrID);
		return CODE_ERR;
	}
    return CODE_OK;
}

//-----------------------------------------------------------------------------
// 								  get_param
//-----------------------------------------------------------------------------
static int get_param(XML_SESSION *sid, const char *ldrID, int *timeout, int *retry, int *retry_wait)
{
	char	xpath_query[255];
	char	tmp_str[255];
	
	sprintf(xpath_query, "/configuration/loader[@name='%s']", ldrID);
	if (lss_xml_get_attr(sid, xpath_query, "timeout", tmp_str, sizeof(tmp_str))==CODE_ERR) return CODE_ERR;
	if (tmp_str) *timeout = atol(tmp_str);
	
	if (lss_xml_get_attr(sid, xpath_query, "retry", tmp_str, sizeof(tmp_str))==CODE_ERR) return CODE_ERR;
	if (tmp_str) *retry = atol(tmp_str);
	
	if (lss_xml_get_attr(sid, xpath_query, "sleep", tmp_str, sizeof(tmp_str))==CODE_ERR) return CODE_ERR;
	if (tmp_str) *retry_wait = atol(tmp_str);
	
	return CODE_OK;
	
}


//-----------------------------------------------------------------------------
//	PRIVATE METHOD
//-----------------------------------------------------------------------------
int start_processor(const char *config_file, const char *ldrID, const char *ProcID, const char *name, 
    const char *date, const char *time, int stream_in, int stream_out)
{
    pid_t pid = fork();
    
	if (pid == (pid_t) -1) {
		CRIT_LOADER_ERROR(name, "fork", 1);	/* ERROR */
    } else if (pid == 0) {    				/* CHILD */
		char host_name[255];

		signal(SIGTERM, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGINT,  SIG_DFL);
		signal(SIGALRM, SIG_DFL); 
	
		if( pgid < 0 ) 
	    	pgid = getpid();
	    
    	if( setpgid(0, pgid) < 0 )
	    	CRIT_LOADER_ERROR(name, "setpgid child", 1);

		if(stream_in != STDIN_FILENO) {
	    	dup2(stream_in,STDIN_FILENO);
	    	close(stream_in);
		}
		if(stream_out != STDOUT_FILENO) {
	    	dup2(stream_out,STDOUT_FILENO);
	    	close(stream_out);
		}
	
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Processor Starting: (modules/%s %s %s %s %s)\n", host_name, getpid(), getppid(), name, config_file, ldrID, ProcID,  date, time);
       
		if (ProcID){
			if (time)	execl(get_module_path(name), name, config_file, ldrID, ProcID, date, time, NULL);
			else		execl(get_module_path(name), name, config_file, ldrID, ProcID, date, NULL);
		}else{
			if (time)	execl(get_module_path(name), name, config_file, ldrID, date, time, NULL);
			else		execl(get_module_path(name), name, config_file, ldrID, date, NULL);
		}
		
		CRIT_LOADER_ERROR(get_module_path(name), "execl", 1);
    } else {    							/* PARENT */
		if( pgid < 0 )
	    	pgid = pid;
        setpgid(pid, pgid);
		if(stream_in != STDIN_FILENO) 
	    	close(stream_in);
		if(stream_out != STDOUT_FILENO) 
	    	close(stream_out);
    }
    return pid;
}
