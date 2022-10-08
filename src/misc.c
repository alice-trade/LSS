#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <assert.h>
#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>

#include <stdlib.h>
#include <stdio.h>

#include "../include/lss_misc.h"

#define	CODE_OK				1
#define	CODE_ERR			0

char 	*config_file 	= NULL;
char	*mgrID 			= NULL;
char 	*ldrID			= NULL;
char 	*procID			= NULL;
char 	*lss_date		= NULL;
char 	*lss_time 		= NULL;

int		check_argv_manager(int argc, char **argv)
{
	char    *tmp_str;
	int		i;

// Step 1: Verify count parametrs
 	if (argc<=1) {
		fprintf(stderr, "Usage: %s mgrID [YYYY-MM-DD [HH24[MI]]]\n", basename(argv[0]));
		return CODE_ERR;
	}
                                                                                                                                                             
// Step 2: Verify and copy DATE parametr
	if (argc>=3) {
		tmp_str = strdup(argv[2]);
		if (strlen(tmp_str)!=10) {
			char host_name[255];

			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Manager Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
			free(tmp_str);
			return CODE_ERR;
		}
		for (i=0;i<strlen(tmp_str);i++) {
			if (!(((tmp_str[i]>='0') && (tmp_str[i]<='9')) || (tmp_str[i]=='-'))) {
				char host_name[255];

				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Manager Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
				free(tmp_str);
				return CODE_ERR;
			}
        }
       	free(tmp_str);
	}
                                                                                                                                                             
// Step 4: Verify and copy TIME parametr
	if (argc==4) {
		tmp_str = strdup(argv[3]);
		if (!(strlen(tmp_str)==2 || strlen(tmp_str)==4)) {
			char host_name[255];
			
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Manager Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
			free(tmp_str);
			return CODE_ERR;
		}
		for (i=0;i<strlen(tmp_str);i++) {
			if (!(tmp_str[i]>='0' && tmp_str[i]<='9')) {
				char host_name[255];
				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Manager Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
				free(tmp_str);
				return CODE_ERR;
			}
		}
		free(tmp_str);
	}
        return CODE_OK;
}

int 	check_argv_loader(int argc, char **argv)
{
	char    *tmp_str;
	int     i;

// Step 1: Verify count parametrs
	if (argc<=2) {
		fprintf(stderr, "Usage: %s ldrID YYYY-MM-DD [HH24[MI]]\n", basename(argv[0]));
		return CODE_ERR;
	}
                                                                                                                                                             
// Step 2: Verify and copy DATE parametr
	tmp_str = strdup(argv[2]);
	if (strlen(tmp_str)!=10) {
		char host_name[255];
		
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Loader Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
		free(tmp_str);
		return CODE_ERR;
	}
	for (i=0;i<strlen(tmp_str);i++) {
		if (!(((tmp_str[i]>='0') && (tmp_str[i]<='9')) || (tmp_str[i]=='-'))) {
			char host_name[255];
			
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Loader Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
			free(tmp_str);
			return CODE_ERR;
		}
	}
	free(tmp_str);
                                                                                                                                                             
// Step 4: Verify and copy TIME parametr
	if (argc==4) {
		tmp_str = strdup(argv[3]);
		if (!(strlen(tmp_str)==2 || strlen(tmp_str)==4)) {
			char host_name[255];
			
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Loader Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
			
			free(tmp_str);
			return CODE_ERR;
		}
		for (i=0;i<strlen(tmp_str);i++) {
			if (!(tmp_str[i]>='0' && tmp_str[i]<='9')) {
				char host_name[255];
				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Loader Error: format time: HH24[MI]\n", host_name, getpid(), getppid());

				free(tmp_str);
				return CODE_ERR;
			}
		}
		free(tmp_str);
	}
	return CODE_OK;
}

int 	check_argv_processor(int argc, char **argv)
{
	char 	*tmp_str;
	int 	i;

// Step 1: Verify count parametrs
	if (argc<=4) {
		fprintf(stderr, "%s starting (build %s/%s)\n",  basename(argv[0]), (__DATE__), (__TIME__));
		fprintf(stderr, "Usage: ./%s config_file ldrID ProcID YYYY-MM-DD [HH24[MI]]\n", basename(argv[0]));
		return CODE_ERR;
	}

// Step 3: Verify and copy DATE	parametr
	tmp_str = strdup(argv[4]);
	if (strlen(tmp_str)!=10) {
		char host_name[255];
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Processor Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
		free(tmp_str);
		return CODE_ERR;
	}
	
	for (i=0;i<strlen(tmp_str);i++) {
		if (!(((tmp_str[i]>='0') && (tmp_str[i]<='9')) || (tmp_str[i]=='-'))) {
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Processor Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());

			free(tmp_str);
			return CODE_ERR;
		}
	}
	free(tmp_str);
	
// Step 4: Verify and copy TIME	parametr
	if (argc==6) {
		tmp_str = strdup(argv[5]);
		
		if (!(strlen(tmp_str)==2 || strlen(tmp_str)==4)) {
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Processor Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
			free(tmp_str);

			return CODE_ERR;
		}
		
		for (i=0;i<strlen(tmp_str);i++) {	// �����������
			if (!(tmp_str[i]>='0' && tmp_str[i]<='9')) {
				char host_name[255];
				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Processor Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
				free(tmp_str);
				return CODE_ERR;
			}
		}
		free(tmp_str);
	}
	return CODE_OK;
}

//-----------------------------------------------------------------------------
// 								  parse_argv_manager
//-----------------------------------------------------------------------------
int
parse_argv_manager(int argc, char **argv, char *mgrID, char *t_date, char *t_time)
{
        char    *tmp_str;
        int             i;

// Step 1: Verify count parametrs
        if (argc<=1) {
                fprintf(stderr, "Usage: %s mgrID [YYYY-MM-DD [HH24[MI]]]\n", basename(argv[0]));
                return CODE_ERR;
        }
                                                                                                                                                             
// Step 2: Verify and copy ldrID parametr
        strncpy(mgrID, argv[1], MAX_SIZE_MGRID);
                                                                                                                                                             
// Step 3: Verify and copy DATE parametr
		if (argc>=3) {
        	tmp_str = strdup(argv[2]);
        	if (strlen(tmp_str)!=10) {
            	    free(tmp_str);
					{
						char host_name[255];
						gethostname(host_name, 255);
						fprintf(stderr,"[%s][%d:%d] Manager Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
					}				
                	return CODE_ERR;
        	}
        	for (i=0;i<strlen(tmp_str);i++) {
                	if (!(((tmp_str[i]>='0') && (tmp_str[i]<='9')) || (tmp_str[i]=='-'))) {
                        	free(tmp_str);
							{
								char host_name[255];
								gethostname(host_name, 255);
								fprintf(stderr,"[%s][%d:%d] Manager Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
							}
                        	return CODE_ERR;
                	}
        	}
        	strncpy(t_date, tmp_str, MAX_SIZE_DATE);
        	free(tmp_str);
		}
                                                                                                                                                             
// Step 4: Verify and copy TIME parametr
        if (argc==4) {
                tmp_str = strdup(argv[3]);
                                                                                                                                                             
                if (!(strlen(tmp_str)==2 || strlen(tmp_str)==4)) {
                        free(tmp_str);
						{
							char host_name[255];
							gethostname(host_name, 255);
							fprintf(stderr,"[%s][%d:%d] Manager Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
						}
                        return CODE_ERR;
                }
                                                                                                                                                             
                for (i=0;i<strlen(tmp_str);i++) {
                        if (!(tmp_str[i]>='0' && tmp_str[i]<='9')) {
                                free(tmp_str);
								{
									char host_name[255];
									gethostname(host_name, 255);
									fprintf(stderr,"[%s][%d:%d] Manager Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
								}
                    	        return CODE_ERR;
                        }
                }
                strncpy(t_time, tmp_str, MAX_SIZE_TIME);
                free(tmp_str);
        }
                                                                                                                                                             
        return CODE_OK;
}


//-----------------------------------------------------------------------------
// 								  parse_argv_loader
//-----------------------------------------------------------------------------
int
parse_argv_loader(int argc, char **argv, char *ldrID, char *date, char *time)
{
        char    *tmp_str;
        int             i;

// Step 1: Verify count parametrs
        if (argc<=2) {
                fprintf(stderr, "Usage: %s ldrID YYYY-MM-DD [HH24[MI]]\n", basename(argv[0]));
                return CODE_ERR;
        }
                                                                                                                                                             
// Step 2: Verify and copy ldrID parametr
        strncpy(ldrID, argv[1], MAX_SIZE_LDRID);
                                                                                                                                                             
// Step 3: Verify and copy DATE parametr
        tmp_str = strdup(argv[2]);
        if (strlen(tmp_str)!=10) {
                free(tmp_str);
				{
					char host_name[255];
					gethostname(host_name, 255);
					fprintf(stderr,"[%s][%d:%d] Loader Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
				}				
                return CODE_ERR;
        }
        for (i=0;i<strlen(tmp_str);i++) {
                if (!(((tmp_str[i]>='0') && (tmp_str[i]<='9')) || (tmp_str[i]=='-'))) {
                        free(tmp_str);
						{
							char host_name[255];
							gethostname(host_name, 255);
							fprintf(stderr,"[%s][%d:%d] Loader Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
						}				
                        return CODE_ERR;
                }
        }
        strncpy(date, tmp_str, MAX_SIZE_DATE);
        free(tmp_str);
                                                                                                                                                             
// Step 4: Verify and copy TIME parametr
        if (argc==4) {
                tmp_str = strdup(argv[3]);
                                                                                                                                                             
                if (!(strlen(tmp_str)==2 || strlen(tmp_str)==4)) {
                        free(tmp_str);
						{
							char host_name[255];
							gethostname(host_name, 255);
							fprintf(stderr,"[%s][%d:%d] Loader Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
						}				
                        return CODE_ERR;
                }
                                                                                                                                                             
                for (i=0;i<strlen(tmp_str);i++) {
                        if (!(tmp_str[i]>='0' && tmp_str[i]<='9')) {
                                free(tmp_str);
								{
									char host_name[255];
									gethostname(host_name, 255);
									fprintf(stderr,"[%s][%d:%d] Loader Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
								}				
                                return CODE_ERR;
                        }
                }
                strncpy(time, tmp_str, MAX_SIZE_TIME);
                free(tmp_str);
        }
                                                                                                                                                             
        return CODE_OK;
}

//-----------------------------------------------------------------------------
// 								  parse_argv_processor
//-----------------------------------------------------------------------------
int 
parse_argv_processor(int argc, char **argv, char *config_file, char *ldrID, char *date, char *time)
{
	char 	*tmp_str;
	int 	i;
/*
	Формат строки:
	config_file ldrID YYYY-MM-DD [HH24[MI]]
	
	Проверки:
	1. Количество параметров, должно быть либо 2, иначе error
*/


// Step 1: Verify count parametrs
	if (argc<=3) {
		fprintf(stderr, "%s starting (build %s/%s)\n",  basename(argv[0]), (__DATE__), (__TIME__));
		fprintf(stderr, "Usage: ./%s config_file ldrID YYYY-MM-DD [HH24[MI]]\n", basename(argv[0]));
		return CODE_ERR;
	}

// Step 2: Verify and copy ldrID and fileconfig parametr
	strncpy(config_file, argv[1], MAX_SIZE_CONFIG);
	strncpy(ldrID, argv[2], MAX_SIZE_LDRID);

// Step 3: Verify and copy DATE	parametr
	tmp_str = strdup(argv[3]);
	if (strlen(tmp_str)!=10) {
		free(tmp_str);
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Processor Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
		}				
		return CODE_ERR;
	}
	
	for (i=0;i<strlen(tmp_str);i++) {
		if (!(((tmp_str[i]>='0') && (tmp_str[i]<='9')) || (tmp_str[i]=='-'))) {
			free(tmp_str);
			{
				char host_name[255];
				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Processor Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
			}				
			return CODE_ERR;
		}
	}
	strncpy(date, tmp_str, MAX_SIZE_DATE);
	free(tmp_str);
	
// Step 4: Verify and copy TIME	parametr
	if (argc==5) {
		tmp_str = strdup(argv[4]);
		
		if (!(strlen(tmp_str)==2 || strlen(tmp_str)==4)) {
			{
				char host_name[255];
				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Processor Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
			}				
			free(tmp_str);
			return CODE_ERR;
		}
		
		for (i=0;i<strlen(tmp_str);i++) {	// исправление
			if (!(tmp_str[i]>='0' && tmp_str[i]<='9')) {
				{
					char host_name[255];
					gethostname(host_name, 255);
					fprintf(stderr,"[%s][%d:%d] Processor Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
				}				
				free(tmp_str);
				return CODE_ERR;
			}
		}
		strncpy(time, tmp_str, MAX_SIZE_TIME);
		free(tmp_str);
	}
	return CODE_OK;
}

//-----------------------------------------------------------------------------
// 								  parse_argv_processor2
//-----------------------------------------------------------------------------
int 
parse_argv_processor2(int argc, char **argv, char *config_file, char *ldrID, char *ProcID, char *date, char *time)
{
	char 	*tmp_str;
	int 	i;
/*
	Формат строки:
	config_file ldrID YYYY-MM-DD [HH24[MI]]
	
	Проверки:
	1. Количество параметров, должно быть либо 2, иначе error
*/


// Step 1: Verify count parametrs
	if (argc<=4) {
		fprintf(stderr, "%s starting (build %s/%s)\n",  basename(argv[0]), (__DATE__), (__TIME__));
		fprintf(stderr, "Usage: ./%s config_file ldrID ProcID YYYY-MM-DD [HH24[MI]]\n", basename(argv[0]));
		return CODE_ERR;
	}

// Step 2: Verify and copy ldrID and fileconfig parametr
	strncpy(config_file, argv[1], MAX_SIZE_CONFIG);
	strncpy(ldrID, argv[2], MAX_SIZE_LDRID);
	strncpy(ProcID, argv[3], MAX_SIZE_LDRID);

// Step 3: Verify and copy DATE	parametr
	tmp_str = strdup(argv[4]);
	if (strlen(tmp_str)!=10) {
		free(tmp_str);
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Processor Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
		}				
		return CODE_ERR;
	}
	
	for (i=0;i<strlen(tmp_str);i++) {
		if (!(((tmp_str[i]>='0') && (tmp_str[i]<='9')) || (tmp_str[i]=='-'))) {
			free(tmp_str);
			{
				char host_name[255];
				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Processor Error: format date: YYYY-MM-DD\n", host_name, getpid(), getppid());
			}				
			return CODE_ERR;
		}
	}
	strncpy(date, tmp_str, MAX_SIZE_DATE);
	free(tmp_str);
	
// Step 4: Verify and copy TIME	parametr
	if (argc==6) {
		tmp_str = strdup(argv[5]);
		
		if (!(strlen(tmp_str)==2 || strlen(tmp_str)==4)) {
			{
				char host_name[255];
				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Processor Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
			}				
			free(tmp_str);
			return CODE_ERR;
		}
		
		for (i=0;i<strlen(tmp_str);i++) {	// исправление
			if (!(tmp_str[i]>='0' && tmp_str[i]<='9')) {
				{
					char host_name[255];
					gethostname(host_name, 255);
					fprintf(stderr,"[%s][%d:%d] Processor Error: format time: HH24[MI]\n", host_name, getpid(), getppid());
				}				
				free(tmp_str);
				return CODE_ERR;
			}
		}
		strncpy(time, tmp_str, MAX_SIZE_TIME);
		free(tmp_str);
	}
	return CODE_OK;
}



//-----------------------------------------------------------------------------
// 								get_xml_value  
//-----------------------------------------------------------------------------
int 
get_xml_value(const char *xpath_query, const char *config_file, char *value) 
{
    xmlDocPtr 			doc;
    xmlXPathContextPtr 	xpathCtx; 
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size;
	
	assert(xpath_query);
	assert(config_file);
	assert(value);
	
	xmlInitParser();
    LIBXML_TEST_VERSION
	
	doc = xmlParseFile(config_file);
    if (doc == NULL) {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: Unable to parse file '%s'\n", host_name, getpid(), getppid(), config_file);
		}				
		return CODE_ERR;
    }

    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: Unable to create new XPath context\n", host_name, getpid(), getppid());
		}				
        xmlFreeDoc(doc); 
        return CODE_ERR;
    }
	
	xpathObj = xmlXPathEvalExpression(xpath_query, xpathCtx);
    
	if(xpathObj == NULL) {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
		}				
        xmlXPathFreeContext(xpathCtx); 
        xmlFreeDoc(doc); 
        return CODE_ERR;
    }
	
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	if (size==1) {
		assert(nodes->nodeTab[0]);
		if(nodes->nodeTab[0]->type == XML_ELEMENT_NODE) {
			strcpy(value, xmlGetProp(nodes->nodeTab[0], "value"));
		}else{
			{
				char host_name[255];
				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Error: section '%s' no correct\n", host_name, getpid(), getppid(), xpath_query);
			}
        	xmlXPathFreeContext(xpathCtx); 
        	xmlFreeDoc(doc); 
        	return CODE_ERR;
		}
	
		xmlXPathFreeContext(xpathCtx); 
		xmlFreeDoc(doc);
    
		xmlCleanupParser();
		xmlMemoryDump();
	}
	return CODE_OK;
}

int 
load_option(const char *name_processor, const char *config_file, const char *ldrID, char *arg_v[])
{
    char 				xpath_expr[255], *str;
    xmlDocPtr 			doc;
    xmlXPathContextPtr 	xpathCtx; 
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size, i;
	int					arg_c = 1;

    	
    assert(config_file);
    assert(ldrID);
	
    xmlInitParser();
    LIBXML_TEST_VERSION

/* Load XML document */
    doc = xmlParseFile(config_file);
    if (doc == NULL) {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: Unable to parse file '%s'\n", host_name, getpid(), getppid(), config_file);
		}				
		return CODE_ERR;
    }

    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: Unable to create new XPath context\n", host_name, getpid(), getppid());
		}				
        xmlFreeDoc(doc); 
        return CODE_ERR;
    }
	sprintf(xpath_expr, "/configuration/loader[@name='%s']/processor[@name='%s']", ldrID, name_processor);

	xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    
	if(xpathObj == NULL) {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_expr);
		}				
        xmlXPathFreeContext(xpathCtx); 
        xmlFreeDoc(doc); 
        return CODE_ERR;
    }
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	xmlXPathFreeObject(xpathObj);
		
	if (size>0) {									
		sprintf(xpath_expr, "/configuration/loader[@name='%s']/processor[@name='%s']/param[@name='options']", ldrID, name_processor);

		xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    
		if(xpathObj == NULL) {
			{
				char host_name[255];
				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_expr);
			}				
        	xmlXPathFreeContext(xpathCtx); 
        	xmlFreeDoc(doc); 
        	return CODE_ERR;
    	}


    	nodes = xpathObj->nodesetval;
		size = (nodes) ? nodes->nodeNr : 0;
    
    	for(i = 0; i < size; ++i) {
			assert(nodes->nodeTab[i]);
			if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
				str = strdup(xmlGetProp(nodes->nodeTab[i], "value"));
				arg_v[arg_c] = str;
				if (arg_c<20) arg_c++;
			}
    	}

		xmlXPathFreeObject(xpathObj);
		xmlXPathFreeContext(xpathCtx); 
	}else {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Loader Error: processor '%s' not found\n", host_name, getpid(), getppid(), name_processor);
		}
		xmlXPathFreeContext(xpathCtx); 
		xmlFreeDoc(doc);
		return CODE_ERR;
	}
	xmlFreeDoc(doc);
    
	xmlCleanupParser();
	xmlMemoryDump();
	arg_v[arg_c] = NULL;
	
    return CODE_OK;
}

/**************************
 Функция для протокола 4
**************************/
int 
load_option2(const char *name_processor, const char *config_file, const char *ldrID, char *arg_v[])
{
    char 				xpath_expr[255], *str;
    xmlDocPtr 			doc;
    xmlXPathContextPtr 	xpathCtx; 
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size, i;
	int					arg_c = 1;

    	
    assert(config_file);
    assert(ldrID);
	
    xmlInitParser();
    LIBXML_TEST_VERSION

/* Load XML document */
    doc = xmlParseFile(config_file);
    if (doc == NULL) {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: Unable to parse file '%s'\n", host_name, getpid(), getppid(), config_file);
		}				
		return CODE_ERR;
    }

    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: Unable to create new XPath context\n", host_name, getpid(), getppid());
		}				
        xmlFreeDoc(doc); 
        return CODE_ERR;
    }
	sprintf(xpath_expr, "/configuration/loader[@name='%s']/processor[@procID='%s']", ldrID, name_processor);

	xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    
	if(xpathObj == NULL) {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_expr);
		}				
        xmlXPathFreeContext(xpathCtx); 
        xmlFreeDoc(doc); 
        return CODE_ERR;
    }
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	xmlXPathFreeObject(xpathObj);
		
	if (size>0) {									
		sprintf(xpath_expr, "/configuration/loader[@name='%s']/processor[@procID='%s']/param[@name='options']", ldrID, name_processor);

		xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    
		if(xpathObj == NULL) {
			{
				char host_name[255];
				gethostname(host_name, 255);
				fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_expr);
			}				
        	xmlXPathFreeContext(xpathCtx); 
        	xmlFreeDoc(doc); 
        	return CODE_ERR;
    	}


    	nodes = xpathObj->nodesetval;
		size = (nodes) ? nodes->nodeNr : 0;
    
    	for(i = 0; i < size; ++i) {
			assert(nodes->nodeTab[i]);
			if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
				str = strdup(xmlGetProp(nodes->nodeTab[i], "value"));
				arg_v[arg_c] = str;
				if (arg_c<20) arg_c++;
			}
    	}

		xmlXPathFreeObject(xpathObj);
		xmlXPathFreeContext(xpathCtx); 
	}else {
		{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Loader Error: processor '%s' not found\n", host_name, getpid(), getppid(), name_processor);
		}
		xmlXPathFreeContext(xpathCtx); 
		xmlFreeDoc(doc);
		return CODE_ERR;
	}
	xmlFreeDoc(doc);
    
	xmlCleanupParser();
	xmlMemoryDump();
	arg_v[arg_c] = NULL;
	
    return CODE_OK;
}

                                                                                                                                                             
void
lss_print_error(const char *type_prog, const char *type_msg, const char *ID, const char *fmt, va_list ap)
{
    char hostname[255];
    gethostname(hostname, 255);
    fprintf (stderr,"\n[%s][%d:%d] %s: %s '%s' ", hostname, getpid(), getppid(), type_msg,type_prog, ID);
    vfprintf(stderr, fmt, ap);
}
		                                                                                                                                                             
void 
processor_print_error(const char *proc, const char *fmt, ...) 
{
    va_list ap;
    va_start(ap, fmt);
    lss_print_error("processor", "Error", proc, fmt, ap);
    va_end(ap);
}

void
processor_print_message(const char *proc, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    lss_print_error("processor", "Info", proc, fmt, ap);
    va_end(ap);
}
