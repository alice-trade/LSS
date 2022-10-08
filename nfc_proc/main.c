/*
Example:
	<param name="directory" value="/nfc/Data"/>
	<param name="agregator" value="DetailInterface"/>
	<param name="router" value="10.100.33.1"/>
*/

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include <libgen.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

#include <zlib.h>

#include <config.h>
#include "../include/lss_misc.h"
#include "../include/lss_conf.h"

#define NAME_PROCESSOR		"nfc_proc"

char		host_name[255];
	
static int save_to_stdout(char *file, FILE *fd_out, FILE *fd_err,  int flag_gz, char *router, char *agrigator, char *type, char *date, char *time);
static int parse_data(FILE*, FILE*, char*, char*, char*, char*, char *t_date, char *p_time);


//--------------------------------------------------------------------------
//			        main
//--------------------------------------------------------------------------

int main(int argc, char **argv)
{
 	char		dir_nfc_root[FILENAME_MAX];
	char		agregator[100], router[50], type[50];
	XML_SESSION	*sid;
	
	DEFINE_ENV_PROCESSOR;
	
	bzero(dir_nfc_root, FILENAME_MAX);
	bzero(agregator, 100);
	bzero(router, 50);
	bzero(type, 50);
	
	gethostname(host_name, 255);
	
	if (check_argv_processor(argc, argv)!=CODE_OK) exit(1);
	
	if ((sid = lss_xml_init(config_file))==NULL) exit(1);
		
	if (lss_xml_get_param_or_null(sid, ldrID, procID, "directory", dir_nfc_root, sizeof(dir_nfc_root)) != CODE_OK) exit(1);
	if (lss_xml_get_param_or_null(sid, ldrID, procID, "agregator", agregator, sizeof(agregator)) != CODE_OK) exit(1);
	if (lss_xml_get_param_or_null(sid, ldrID, procID, "router", router, sizeof(router)) != CODE_OK) exit(1);
	if (lss_xml_get_param_or_null(sid, ldrID, procID, "type", type, sizeof(type)) != CODE_OK) exit(1);
	
	lss_xml_free(sid);
	
	if (parse_data(stdout, stderr, dir_nfc_root, router, agregator, type, lss_date, lss_time) != CODE_OK) exit(1);
 
 	exit(0);
}

//------------------------------------------------------------------------------
//				parse_data
//------------------------------------------------------------------------------
int             
parse_data(FILE *fd_out, FILE *fd_err, char *dir, char *t_router, char *t_aggregator, char *t_type, char *p_date, char *t_time) 
{
 	char		agrigator[100];
 	char		router[100];
 	char		time[100];
 
 	DIR    		*dd_1, *dd_2, *dd_3;
 	struct 		dirent 	*dp_1, *dp_2, *dp_3;
 	char		name_dir_1[FILENAME_MAX], name_dir_2[FILENAME_MAX], name_dir_3[FILENAME_MAX];
 	char		*pos, *s_dir, *t_date, *nfile;
 	int			i, flag_gz = 0;
	char		host_name[255];


 
/* 
	Рекомендуемая структура nfc-дерева:

	root
		date
			router
				agregator type		
					data file	
*/
	
	s_dir = (char*) malloc(FILENAME_MAX);
	t_date = strdup(p_date);

	for(i=0;i<strlen(t_date);i++) if (t_date[i]=='-') t_date[i]='_';

	sprintf(s_dir, "%s/%s", dir, t_date);

	if ((dd_1 = opendir(s_dir)) != NULL) {
  		while ((dp_1 = readdir(dd_1)) != NULL) {
   			if ((strcmp(dp_1->d_name, "..") != 0) && (strcmp(dp_1->d_name, ".") != 0)) {
				sprintf(router, "%s", dp_1->d_name);
    			sprintf(name_dir_1, "%s/%s", s_dir, dp_1->d_name); 
// 1 level - routers
				if (!*t_router || strncmp(dp_1->d_name, t_router, 50)==0){
    				if ((dd_2 = opendir(name_dir_1)) != NULL) {
     					while ((dp_2 = readdir(dd_2)) != NULL) {
      						if ((strcmp(dp_2->d_name, "..") != 0) && (strcmp(dp_2->d_name, ".") != 0)) {
								sprintf(agrigator, "%s", dp_2->d_name);
      							sprintf(name_dir_2, "%s/%s", name_dir_1, dp_2->d_name);

								if (!*t_aggregator || strncmp(dp_2->d_name, t_aggregator, 100)==0) {
// 2 level - agregating type
       								if ((dd_3 = opendir(name_dir_2)) != NULL) {
	        							while ((dp_3 = readdir(dd_3)) != NULL) {
         									if ((strcmp(dp_3->d_name, "..") != 0) && (strcmp(dp_3->d_name, ".") != 0)) {
          										sprintf(name_dir_3, "%s/%s", name_dir_2, dp_3->d_name);
// 3 level - data files	
												nfile = strdup(name_dir_3);				// делаем копию пути файла
													if (strstr(nfile, "gz")) {			// ищем подстроку gz, если есть, то выставляем флаг архива
														if ((pos = strrchr(nfile, '.'))) {
   															*pos = '\0';
															flag_gz = 1;
   														}else {
															
		   													fprintf(fd_err, "[%s][%d:%d] Error: strrchr(gz)\n", host_name, getpid(), getppid());
															free(nfile);
															free(s_dir);
															free(t_date);
   															return CODE_ERR;
   														}
													}else flag_gz = 0;
													if ((pos = strrchr(nfile, '.'))) {	// находим время
														pos++;
														sprintf(time, "%s", pos); 
													}else {
														fprintf(fd_err, "[%s][%d:%d] Error:  strrchr(time)\n", host_name, getpid(), getppid());
														free(nfile);
														free(t_date);
														free(s_dir);
														return CODE_ERR;
													}
											
													time[8] = '\0'; time[7] = '0'; time[6] = '0'; time[5] = ':'; time[4] = time[3]; time[3] = time[2]; time[2] = ':';
													if (t_time){

														if (strlen(t_time)==4) {
															if (strncmp(pos, t_time, 3)==0) {
																if (save_to_stdout(name_dir_3, fd_out, fd_err, flag_gz, router, agrigator, t_type, p_date, time)!=CODE_OK) return CODE_ERR;
															}
														}else if (strlen(t_time)==2){
															if (strncmp(pos, t_time, 2)==0) {
																if (save_to_stdout(name_dir_3, fd_out, fd_err, flag_gz, router, agrigator, t_type, p_date, time)!=CODE_OK) return CODE_ERR;
															}
														}														
													}else {

														if (save_to_stdout(name_dir_3, fd_out, fd_err, flag_gz, router, agrigator, t_type, p_date, time)!=CODE_OK) return CODE_ERR;
													}
												free(nfile);
											}
	   									}								
       								}else { 
										fprintf(fd_err, "[%s][%d:%d] Error: 3 level error opendir(): %s\n", host_name, getpid(), getppid(), strerror(errno)); 
										free(s_dir);
										free(t_date);
										return CODE_ERR; 
									}
								}
      						}
     					}
    				}else 
					{ 
						fprintf(fd_err, "[%s][%d:%d] Error: 2 level error opendir(): %s\n", host_name, getpid(), getppid(), strerror(errno)); 
						free(s_dir);
						free(t_date);
						return CODE_ERR; 
					}
				}
   			}
  		}
 	}else { 
		fprintf(fd_err, "[%s][%d:%d] Error: 1 level error open dir :%s (%s)\n", host_name, getpid(), getppid(), s_dir, strerror(errno)); 
		free(s_dir);
		free(t_date);
		return CODE_ERR; 
	}
 	fflush(fd_out);
	
	return CODE_OK;
}


static int 
save_to_stdout(char *file, FILE *fd_out, FILE *fd_err, int flag_gz, char *router, char *agrigator, char *type, char *date, char *time)
{
	gzFile 		*fd_gz; 
	FILE		*fd;	
	char		*buffer;
	
	buffer = (char*) malloc(1024);

	if (flag_gz) {
		if ((fd_gz = gzopen (file, "r"))!=NULL) {             
    		fprintf(fd_out, "# BEGIN BLOCK #\n");
        	fprintf(fd_out, "# DATE %10s %8s #\n", date, time);
        	fprintf(fd_out, "# ROUTER %s #\n", router);
        	fprintf(fd_out, "# AGGREGATION %s #\n", agrigator);
	
        	while(gzgets(fd_gz, buffer, 1024)) {
	         	if(*buffer && buffer[0]!='#' && buffer[0]!='\n') {
         			if (*type == '\0' || strcmp(type, "nfc")==0) {
						if (buffer[0] >= '0' && buffer[0] <= '9') fprintf(fd_out, "%s", buffer);
					}else{
						fprintf(fd_out, "%s", buffer);
					}
	     		}
	    	}
	    	gzclose(fd_gz);	      
	    	fprintf(fd_out, "# END BLOCK #\n\n");
		}else { 
			fprintf(fd_err, "[%s][%d:%d] Error: open gzip file '%s': error - '%s'\n", host_name, getpid(), getppid(), file, strerror(errno)); 
			free(buffer);
			return CODE_ERR; 
		}
	}else{
		if ((fd = fopen (file, "r"))!=NULL) {             
    		fprintf(fd_out, "# BEGIN BLOCK #\n");
        	fprintf(fd_out, "# DATE %10s %8s #\n", date, time);
        	fprintf(fd_out, "# ROUTER %s #\n", router);
        	fprintf(fd_out, "# AGGREGATION %s #\n", agrigator);
	
        	while(fgets(buffer, 1024, fd)) {
	         	if(*buffer && buffer[0]!='#' && buffer[0]!='\n') {
         			if (*type == '\0' || strcmp(type, "nfc")==0) {
						if (buffer[0] >= '0' && buffer[0] <= '9') fprintf(fd_out, "%s", buffer);
					}else{
						fprintf(fd_out, "%s", buffer);
					}
					
	     		}
	    	}
	    	fclose(fd);	      
	    	fprintf(fd_out, "# END BLOCK #\n\n");
		}else { 
			fprintf(fd_err, "[%s][%d:%d] Error: open file '%s': error - '%s'\n", host_name, getpid(), getppid(), file, strerror(errno)); 
			free(buffer);
			return CODE_ERR; 
		}
		
	}
	free(buffer);
	return CODE_OK;
}
