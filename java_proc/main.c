/* Created by Anjuta version 1.2.2 */
/*	This file will not be overwritten */
/*
	/configuration/loader[@name='%s']/processor[@name='%s']/param[@name='options']" -> value

*/
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

#include <config.h>

#include "../include/lss_misc.h"
#include "../include/lss_conf.h"

#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>

char 	*NAME_PROCESSOR;
char 	host_name[255];

//-----------------------------------------------------------------------------
//									BEGIN PROGRAM
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int			i;
    char 		*arg_v[30];
	XML_SESSION	*sid;
	char		prog_name[50];
	
	DEFINE_ENV_PROCESSOR;
	bzero(prog_name, sizeof(prog_name));
	NAME_PROCESSOR = strdup(basename(argv[0]));
	gethostname(host_name, 255);
	
	if (check_argv_processor(argc, argv)!=CODE_OK) exit(1);
	
	if ((sid = lss_xml_init(config_file)) == NULL) exit(1);
	
	arg_v[0] = strdup(NAME_PROCESSOR);
	if (lss_xml_get_param_or_null(sid, ldrID, procID, "program", prog_name, sizeof(prog_name)) != CODE_OK) exit(1);
	if (lss_xml_get_options(sid, ldrID, procID, 1, arg_v, 20)!=CODE_OK) exit(1);
	
	for(i=0;i<30;i++){
		if (arg_v[i] == NULL) break;
	}

	arg_v[i++] = strdup(config_file);
	arg_v[i++] = strdup(ldrID);
	arg_v[i++] = strdup(procID);	
	arg_v[i++] = strdup(lss_date);
	if (lss_time!=NULL) arg_v[i++] = strdup(lss_time);
	arg_v[i] = NULL;
	
	lss_xml_free(sid);
	
	if (*prog_name) execvp(prog_name, arg_v);
	else			execvp("java", arg_v);
	   
	CRIT_PROC_ERROR(NAME_PROCESSOR, "execvp", 1);
}
