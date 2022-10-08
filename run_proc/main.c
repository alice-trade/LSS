/* Created by Anjuta version 1.2.2 */
/*	This file will not be overwritten */
/*	/configuration/loader[@name='%s']/processor[@name='%s']/param[@name='option']
	/configuration/loader[@name='%s']/processor[@name='%s']/param[@name='program']
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

char *NAME_PROCESSOR;
char host_name[255];


//-----------------------------------------------------------------------------
//				BEGIN PROGRAM
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    char 		*arg_v[20];
	char		prog_name[50], buf[1024];
	XML_SESSION	*sid;
	
	DEFINE_ENV_PROCESSOR;
	
	NAME_PROCESSOR = strdup(basename(argv[0]));	
	gethostname(host_name, 255);
	
	if (check_argv_processor(argc, argv)!=CODE_OK) exit(1);

	if ((sid = lss_xml_init(config_file)) == NULL) exit(1);
		
	if (lss_xml_get_param(sid, ldrID, procID, "program", prog_name, sizeof(prog_name)) != CODE_OK) exit(1);
	
	if (!*prog_name) {
		fprintf(stderr, "[%s][%d:%d] Error: not found section '%s' \n", host_name, getpid(), getppid(), buf);
		exit(1);
	}		
	
	arg_v[0] = strdup(procID);
	
	if (lss_xml_get_options(sid, ldrID, procID, 1, arg_v, 20)!=CODE_OK) exit(1);
	
	lss_xml_free(sid);
	
	execvp(prog_name, arg_v);
	CRIT_PROC_ERROR(NAME_PROCESSOR, "execvp", 1);
}
