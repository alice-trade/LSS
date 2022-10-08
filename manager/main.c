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
#include "../include/lss_conf.h"

int 	children 		= 0;
int 	children_mgr 	= 0;

char 	host_name[255];
char	log_path[FILENAME_MAX];
char	modules_dir[FILENAME_MAX];
int 	pipe_channel[2];
char	t_date[20], t_time[20];

static int load_loader(XML_SESSION *sid, const char *mgrID, char *t_date, char *t_time);
static int load_manager(XML_SESSION *sid, const char *mgrID, char *t_date, char *t_time);
static int calc_time(int, char);
static int send_notify(const int EXIT_STATUS, XML_SESSION *sid, const char *config_file, const char *mgrID, char *t_date, char *t_time);

int	start_notify(int *stream_in, int *stream_out, const int EXIT_STATUS, const char *config_file, const char *mgrID, const char *notifyID, const char *typeID, const char *m_date, const char *m_time);
	
int 
start_loader(const char *name, const char *t_date, const char *t_time) 
{
    pid_t 	p;
	char	loader_path[FILENAME_MAX];

    p = fork();
    
    if (p == (pid_t) -1) {
		CRIT_MANAGER_ERROR("lss_mgr", "fork", 1);		/* ERROR */
    } else if (p == 0) {    	/* CHILD */
		char host_name[255];
		
		gethostname(host_name, 255);
		printf("Starting Loader [%s][%d] %s\n",host_name, getpid(), name);
		fflush(stdout);
		
		sprintf(loader_path, "%s/%s", LOADER_DIR, "lss_ldr");
		if (!t_date) {
			execl(loader_path, "lss_ldr", name, NULL);
			CRIT_MANAGER_ERROR("lss_mgr", "execl", 1);
		}else if (!t_time) {
			execl(loader_path, "lss_ldr", name, t_date, NULL);
			CRIT_MANAGER_ERROR("lss_mgr", "execl", 1);
		} else {
			execl(loader_path, "lss_ldr", name, t_date, t_time, NULL);
			CRIT_MANAGER_ERROR("lss_mgr", "execl", 1);
		}
    } else {                	/* PARENT */
		children++;
    }
	return 0;
}

int 
start_manager(const char *name, const char *t_date, const char *t_time) 
{
    pid_t 	p;
	char	loader_path[FILENAME_MAX];

    p = fork();
    
    if (p == (pid_t) -1) {
		CRIT_MANAGER_ERROR("lss_mgr", "fork", 1);		/* ERROR */
    } else if (p == 0) {    	/* CHILD */
		char host_name[255];
		
		gethostname(host_name, 255);
		printf("Starting Manager [%s][%d] %s\n",host_name, getpid(), name);
		fflush(stdout);
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
    } else {                	/* PARENT */
		children_mgr++;
    }
	return 0;
}

int
start_notify(int *stream_in, int *stream_out, const int EXIT_STATUS, const char *config_file, const char *mgrID, const char *notifyID, const char *typeID, const char *m_date, const char *m_time)
{
    pid_t 	p;
	char	path[FILENAME_MAX];

    p = fork();
    
    if (p == (pid_t) -1) {
		CRIT_MANAGER_ERROR("lss_mgr", "fork", 1);		/* ERROR */
    } else if (p == 0) {    	/* CHILD */
		char 	host_name[255];
		char	exit_status[5];
		
		gethostname(host_name, 255);
		sprintf(exit_status, "%d", EXIT_STATUS);
		sprintf(path, "%s/%s_notify", modules_dir, typeID);
		

		close(stream_out[0]);
		close(stream_in[1]);
		
		
		if(stream_in[0] != STDIN_FILENO) {
	    	dup2(stream_in[0],STDIN_FILENO);
	    	close(stream_in[0]);
		}
		if(stream_out[1] != STDOUT_FILENO) {
	    	dup2(stream_out[1],STDOUT_FILENO);
	    	close(stream_out[1]);
		}
		
		if (!m_date) {
			execl(path, basename(path), exit_status, log_path, mgrID, NULL);
			CRIT_MANAGER_ERROR("lss_mgr [notify]", "execl", 1);
		}else if (!m_time) {
			execl(path, basename(path),  exit_status, log_path, mgrID, m_date, NULL);
			CRIT_MANAGER_ERROR("lss_mgr [notify]", "execl", 1);
		} else {
			execl(path, basename(path),  exit_status, log_path, mgrID, m_date, m_time, NULL);
			CRIT_MANAGER_ERROR("lss_mgr [notify]", "execl", 1);
		}
    }else{
		if(stream_in[0] != STDIN_FILENO) 
	    	close(stream_in[0]);
		if(stream_out[1] != STDOUT_FILENO) 
	    	close(stream_out[1]);
	}
	return 0;	
}

int 
start_logger(char *mgrID, char *dir) 
{
	struct tm       td;
	time_t 			s_time;
	int				fd_log;
	
	bzero(log_path, FILENAME_MAX);
	
	s_time = time(NULL);
	localtime_r(&s_time, &td);
	sprintf(log_path, "%s/%s_%04d-%02d-%02d:%02d:%02d:%02d",  dir, mgrID, td.tm_year-100+2000, td.tm_mon+1, td.tm_mday, td.tm_hour, td.tm_min, td.tm_sec);
	
	if ((fd_log = open(log_path, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR))==-1) {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr, "Manager Error [%s][%d]: error open log file '%s'\n", host_name, getpid(), log_path);
		}
		return CODE_ERR;
	}
	dup2(fd_log, STDOUT_FILENO);
	dup2(fd_log, STDERR_FILENO);
	
	return CODE_OK;
}

int 
main(int argc, char *argv[]) 
{
    int 			pid, status, result = 0;
	char			dir_tmp[FILENAME_MAX];
	char			str_period[50];
	int				time_volume;
	char			time_mn;
	char			xpath_query[2024];
	XML_SESSION		*sid;
	
	DEFINE_ENV_MANAGER;
	
	bzero(str_period, 50);
	bzero(xpath_query, 2024);
	bzero(log_path, FILENAME_MAX);
	
	gethostname(host_name, 255);
	
	if (check_argv_manager(argc, argv)!=CODE_OK) exit(1);
	
	if ((sid = lss_xml_init(CONFIG_FILE))==NULL) exit(1);
		
	sprintf(xpath_query, "/configuration/manager[@name='%s']/period", mgrID);
	if (lss_xml_get_value(sid, xpath_query, str_period, sizeof(str_period))!=CODE_OK) exit(1);
	
	if (lss_xml_get_value(sid, "/configuration/directory/processors[@value]", modules_dir, sizeof(modules_dir))!=CODE_OK) exit(1);

	if (!*str_period){
		fprintf(stderr, "[%s][%d] Manager Error: not found period or invalid section (/configuration/manager[@name='%s']/period) (%s)\n", host_name, getpid(), mgrID, mgrID);
		exit(1);
	}
	
	if (sscanf(str_period, "%d%c", &time_volume, &time_mn)!=2) {
		fprintf(stderr, "[%s][%d] Manager Error: error sscanf (%s)\n", host_name, getpid(), mgrID);
		exit(1);
	}
	
	if (!lss_date) {		// Если дата не заданна, то ...
		struct tm       td;
		time_t 			s_time;
		
		
		s_time = time(NULL);
		s_time-=calc_time(time_volume, time_mn);
		
		localtime_r(&s_time, &td);
		
		sprintf(t_date, "%04d-%02d-%02d",  td.tm_year-100+2000, td.tm_mon+1, td.tm_mday);
		lss_date = t_date;
	}
	
	if (!lss_time) {		// Если время не заданно, то ...
		struct tm       td;
		time_t 			s_time;
		
		if (time_mn =='m') {
			
			s_time = time(NULL);
			s_time-=calc_time(time_volume, time_mn);
			
			localtime_r(&s_time, &td);
			sprintf(t_time, "%02d%02d",  td.tm_hour, td.tm_min);
			lss_time = t_time;
		}else if (time_mn =='h') {
			
			s_time = time(NULL);
			s_time-=calc_time(time_volume, time_mn);
			
			localtime_r(&s_time, &td);
			sprintf(t_time, "%02d",  td.tm_hour);
			lss_time = t_time;
		}

	}

	if (lss_xml_get_value(sid, "/configuration/directory/log", dir_tmp, sizeof(dir_tmp))!=CODE_OK) exit(1);

   	if (start_logger(mgrID, dir_tmp)!=CODE_OK) exit(1);
	
    printf("[%s][%d] Start LSS Manager (%s)\n", host_name, getpid(), mgrID);
	fflush(stdout);
	
	if (load_loader(sid, mgrID, lss_date, lss_time)!=CODE_OK) exit(1);
	
	status = 0;
    for(;;) {
		pid = waitpid(-1, &status, 0);
		if( pid == -1 && errno == EINTR )  continue;
		printf("[%s][%d] Stop Loader exit with %d code result\n", host_name, pid, status);
		fflush(stdout);

		result = result || (status != 0);
		if( pid < 0 || ! --children )  break;
    }

	if (result != 0) {
		goto l_exit;
	}
	
	if (load_manager(sid, mgrID, lss_date, lss_time)!=CODE_OK) exit(1);
	
	status = 0;
    if (children_mgr>0){
		for(;;) {
			pid = waitpid(-1, &status, 0);
			if( pid == -1 && errno == EINTR )  continue;
			printf("[%s][%d] Stop Manager exit with %d code result\n", host_name, pid, status);
			fflush(stdout);

			result = result || (status != 0);
			if( pid < 0 || ! --children_mgr )  break;
    	}
	}

l_exit:
    printf("[%s][%d] LSS Manager (%s) exit with %d code result\n", host_name, getpid(), mgrID, result);
	fflush(stdout);
	
	send_notify(result, sid, CONFIG_FILE, mgrID, lss_date, lss_time);
	
	lss_xml_free(sid);
    exit(result!=0);
}

static int 
calc_time(int vol_time, char mn)
{
	int	tmp_vol = 0;
	
	if (mn == 'd'){
		tmp_vol = vol_time*3600*24;	
	}else if (mn == 'h'){
		tmp_vol = vol_time*3600;
	}else if (mn == 'm'){
		tmp_vol = vol_time*60;
	}
	return tmp_vol;
}

static int 
load_loader(XML_SESSION *sid, const char *mgrID, char *t_date, char *t_time)
{
    char 				xpath_query[255];
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size, i;
    	
	sprintf(xpath_query, "/configuration/manager[@name='%s']", mgrID);

	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
	if(xpathObj == NULL) {
		fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
        return CODE_ERR;
    }
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	xmlXPathFreeObject(xpathObj);
		
	if (size>0) {
		sprintf(xpath_query, "/configuration/manager[@name='%s']/loader", mgrID);

		xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
		if(xpathObj == NULL) {
			fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
        	return CODE_ERR;
    	}


    	nodes = xpathObj->nodesetval;
		size = (nodes) ? nodes->nodeNr : 0;
    
    	for(i = 0; i < size; ++i) {
			assert(nodes->nodeTab[i]);
			if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {	// element node?
	    		if (start_loader(xmlGetProp(nodes->nodeTab[i], "name"), t_date, t_time)<0){
					fprintf(stderr,"[%s][%d:%d] Manager Error: error start loader '%s'\n", host_name, getpid(), getppid(), xmlGetProp(nodes->nodeTab[i], "name"));
					return CODE_ERR;
	    		}
			}
    	}

	}else {
		char host_name[255];

		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Manager Error: manager '%s' not found\n", host_name, getpid(), getppid(), mgrID);
		return CODE_ERR;
	}
	xmlXPathFreeObject(xpathObj);
    return CODE_OK;
}

static int 
load_manager(XML_SESSION *sid, const char *mgrID, char *t_date, char *t_time)
{
    char 				xpath_query[255];
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size, i;
    	
	sprintf(xpath_query, "/configuration/manager[@name='%s']", mgrID);

	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
	if(xpathObj == NULL) {
		fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
        return CODE_ERR;
    }
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	xmlXPathFreeObject(xpathObj);
		
	if (size>0) {
		sprintf(xpath_query, "/configuration/manager[@name='%s']/manager", mgrID);

		xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
		if(xpathObj == NULL) {
			fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
        	return CODE_ERR;
    	}

    	nodes = xpathObj->nodesetval;
		size = (nodes) ? nodes->nodeNr : 0;
    
    	for(i = 0; i < size; ++i) {
			if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {	// element node?
	    		if (start_manager(xmlGetProp(nodes->nodeTab[i], "name"), t_date, t_time)<0){
					fprintf(stderr,"[%s][%d:%d] Manager Error: error start manager '%s'\n", host_name, getpid(), getppid(), xmlGetProp(nodes->nodeTab[i], "name"));
					return CODE_ERR;
	    		}
			}
    	}

	}else {
		char host_name[255];
	
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Manager Error: manager '%s' not found\n", host_name, getpid(), getppid(), mgrID);
		return CODE_ERR;
	}
	xmlXPathFreeObject(xpathObj);
		
    return CODE_OK;
}

static int
parse_notify(const int EXIT_STATUS, const char *config_file, const char *mgrID, const char *notifyID, const char *t_date, const char *t_time)
{
    char 				xpath_expr[255];
    xmlDocPtr 			doc;
    xmlXPathContextPtr 	xpathCtx; 
    xmlXPathObjectPtr 	xpathObj, xpathObj_Data; 
    xmlNodeSetPtr 		nodes;
    int 				size, i;
    	
    assert(config_file);
    assert(mgrID);

    xmlInitParser();
    LIBXML_TEST_VERSION

/* Load XML document */
    doc = xmlParseFile(config_file);
    if (doc == NULL) {
		fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to parse file '%s'\n", host_name, getpid(), getppid(), config_file);
		return CODE_ERR;
    }

    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
		fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to create new XPath context\n", host_name, getpid(), getppid());
        xmlFreeDoc(doc); 
        return CODE_ERR;
    }
	sprintf(xpath_expr, "/configuration/notify[@name='%s']", notifyID);

	xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    
	if(xpathObj == NULL) {
		fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_expr);
        xmlXPathFreeContext(xpathCtx); 
        xmlFreeDoc(doc); 
        return CODE_ERR;
    }
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	xmlXPathFreeObject(xpathObj);
		
	if (size>0) {
		sprintf(xpath_expr, "/configuration/notify[@name='%s']", notifyID);

		xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    
		if(xpathObj == NULL) {
			fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_expr);
        	xmlXPathFreeContext(xpathCtx); 
        	xmlFreeDoc(doc); 
        	return CODE_ERR;
    	}


    	nodes = xpathObj->nodesetval;
		size = (nodes) ? nodes->nodeNr : 0;
    
    	for(i = 0; i < size; ++i) {
			assert(nodes->nodeTab[i]);
			if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {	// element node?
				char 	*type;
				int		ok_flag = 0;
				
				type = strdup(xmlGetProp(nodes->nodeTab[i], "type"));
				if (strcmp(xmlGetProp(nodes->nodeTab[i], "on_ok"), "yes")==0) ok_flag = 1;else ok_flag=0;
					
				if (EXIT_STATUS || ok_flag){
					xmlNodeSetPtr 		nodes_data;
    				int 				size_data, i_data;
					
					sprintf(xpath_expr, "/configuration/notify[@name='%s']/data", notifyID);
					xpathObj_Data = xmlXPathEvalExpression(xpath_expr, xpathCtx);
					nodes_data = xpathObj_Data->nodesetval;
					size_data = (nodes_data) ? nodes_data->nodeNr : 0;

					if (size_data>0){
						int 	stream_in[2];
						int		stream_out[2];
						int		fd_in, fd_out;
						FILE *fd_stream;
						
						if (pipe(stream_in)) CRIT_PROC_ERROR(procID, "create_pipe_in", errno);	// Создаём pipe
						if (pipe(stream_out)) CRIT_PROC_ERROR(procID, "create_pipe_out", errno);	// Создаём pipe							
						
						fd_in	= stream_out[0];
						fd_out  = stream_in[1];
						
						
						start_notify(stream_in, stream_out, EXIT_STATUS, config_file, mgrID, notifyID, type, t_date, t_time);
  						
						fd_stream = fdopen(fd_out, "w");
  						for (i_data=0;i_data<size_data;++i_data){
							fprintf(fd_stream, "%s\n", xmlNodeGetContent(nodes_data->nodeTab[i_data]));
						}
						fclose (fd_stream);
						for(;;) {
							ssize_t size;
							char	buf[1024];
					
							bzero(buf, sizeof(buf));
		    				size = read(fd_in, buf, sizeof(buf));
		    				if (size==0) break;
		    				if (size==-1) CRIT_PROC_ERROR(procID, "read for pipe", errno);
						}
						close(fd_in);
					}
					xmlXPathFreeObject(xpathObj_Data);
				}
				free(type);
			}
    	}

		xmlXPathFreeObject(xpathObj);
		xmlXPathFreeContext(xpathCtx); 
	}else {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Manager Error: notify '%s' not found\n", host_name, getpid(), getppid(), notifyID);
		}
		xmlXPathFreeContext(xpathCtx); 
		xmlFreeDoc(doc);
		return CODE_ERR;
	}
	xmlFreeDoc(doc);
    
	xmlCleanupParser();
	xmlMemoryDump();
		
    return CODE_OK;
}

static int 
send_notify(const int EXIT_STATUS, XML_SESSION *sid, const char *config_file, const char *mgrID, char *t_date, char *t_time) 
{
    char 				xpath_expr[255];
    xmlDocPtr 			doc;
    xmlXPathContextPtr 	xpathCtx; 
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size, i;
    	
    xmlInitParser();
    LIBXML_TEST_VERSION

/* Load XML document */
    doc = xmlParseFile(config_file);
    if (doc == NULL) {
		fprintf(stderr,"[%s][%d:%d] Manager Error: (send_notify) Unable to parse file '%s'\n", host_name, getpid(), getppid(), config_file);
		return CODE_ERR;
    }

    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
		fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to create new XPath context\n", host_name, getpid(), getppid());
        xmlFreeDoc(doc); 
        return CODE_ERR;
    }
	sprintf(xpath_expr, "/configuration/manager[@name='%s']", mgrID);

	xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    
	if(xpathObj == NULL) {
		fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_expr);
        xmlXPathFreeContext(xpathCtx); 
        xmlFreeDoc(doc); 
        return CODE_ERR;
    }
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	xmlXPathFreeObject(xpathObj);
		
	if (size>0) {
		sprintf(xpath_expr, "/configuration/manager[@name='%s']/notify", mgrID);

		xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    
		if(xpathObj == NULL) {
			fprintf(stderr,"[%s][%d:%d] Manager Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_expr);
        	xmlXPathFreeContext(xpathCtx); 
        	xmlFreeDoc(doc); 
        	return CODE_ERR;
    	}


    	nodes = xpathObj->nodesetval;
		size = (nodes) ? nodes->nodeNr : 0;
    
    	for(i = 0; i < size; ++i) {
			assert(nodes->nodeTab[i]);
			if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {	// element node?
	    		if (parse_notify(EXIT_STATUS, config_file, mgrID, xmlGetProp(nodes->nodeTab[i], "name"), t_date, t_time)<0){
					fprintf(stderr,"[%s][%d:%d] Manager Error: error start loader '%s'\n", host_name, getpid(), getppid(), xmlGetProp(nodes->nodeTab[i], "name"));
					return CODE_ERR;
	    		}
			}
    	}

		xmlXPathFreeObject(xpathObj);
		xmlXPathFreeContext(xpathCtx); 
	}else {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Manager Error: manager '%s' not found\n", host_name, getpid(), getppid(), mgrID);
		}
		xmlXPathFreeContext(xpathCtx); 
		xmlFreeDoc(doc);
		return CODE_ERR;
	}
	xmlFreeDoc(doc);
    
	xmlCleanupParser();
	xmlMemoryDump();
		
    return CODE_OK;
}
