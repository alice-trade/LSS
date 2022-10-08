#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "../include/lss_conf.h"

#define CODE_ERR	0
#define	CODE_OK		1


//-----------------------------------------------------------------------------
// 								lss_xml_init
//-----------------------------------------------------------------------------
XML_SESSION
*lss_xml_init(const char *config_file) 
{
	XML_SESSION *sid = NULL;

	if ((sid = (XML_SESSION *) malloc(sizeof(XML_SESSION)))==NULL) {
		char 	host_name[255];
		
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] error allocate memory for struct XML_SESSION\n", host_name, getpid(), getppid());
		return NULL;
	}
	bzero(sid, sizeof(XML_SESSION));
	
	xmlInitParser();
	
	sid->config_file 	= config_file;
	sid->doc 			= xmlParseFile(config_file);
    
	if (sid->doc == NULL) {
		char 	host_name[255];
		
		gethostname(host_name, 255);		
		fprintf(stderr,"[%s][%d:%d] Error: Unable to parse file '%s'\n", host_name, getpid(), getppid(), config_file);
		free(sid);
		return NULL;
    }

    sid->xpathCtx = xmlXPathNewContext(sid->doc);
    if(sid->xpathCtx == NULL) {
		char 	host_name[255];
		
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: Unable to create new XPath context\n", host_name, getpid(), getppid());
		free(sid);
        xmlFreeDoc(sid->doc); 
        return NULL;
    }

	return sid;
}

//-----------------------------------------------------------------------------
// 								lss_xml_free
//-----------------------------------------------------------------------------
int 
lss_xml_free(XML_SESSION *sid) 
{
	xmlFreeDoc(sid->doc);
	xmlXPathFreeContext(sid->xpathCtx); 
	    
	xmlCleanupParser();
	xmlMemoryDump();
	
	free(sid);
	sid = NULL;
	
	return CODE_OK;
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_param (v.4)
//-----------------------------------------------------------------------------
int 
lss_xml_get_param(XML_SESSION *sid, const char *ldrID, const char *procID, char const *param, char *value, size_t size_value)
{
	char				xpath_query[255];
	xmlXPathObjectPtr   xpathObj;
	xmlNodeSetPtr		nodes;
	int					size;
	
	if (sid == NULL) return CODE_ERR;
		
	sprintf(xpath_query, "/configuration/loader[@name='%s']/processor[@procID='%s']/param[@name='%s']", ldrID, procID, param);

	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
	
	if(xpathObj == NULL) {
		char 	host_name[255];
		
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
		return CODE_ERR;
	}
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	
	if (size==1){
		if(nodes->nodeTab[0]->type == XML_ELEMENT_NODE)
		strncpy(value, xmlGetProp(nodes->nodeTab[0], "value"), size_value);
	} else {
		char 	host_name[255];
		
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: processor procID='%s' return null result or multiline value\n", host_name, getpid(), getppid(), procID);
		xmlXPathFreeObject(xpathObj);
		return CODE_ERR;
	}
	xmlXPathFreeObject(xpathObj);
	return CODE_OK;
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_param_or_null (v.4)
//-----------------------------------------------------------------------------
int 
lss_xml_get_param_or_null(XML_SESSION *sid, const char *ldrID, const char *procID, char const *param, char *value, size_t size_value)
{
	char				xpath_query[255];
	xmlXPathObjectPtr   xpathObj;
	xmlNodeSetPtr		nodes;
	int					size;
	
	if (sid == NULL) return CODE_ERR;
		
	sprintf(xpath_query, "/configuration/loader[@name='%s']/processor[@procID='%s']/param[@name='%s']", ldrID, procID, param);

	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
	
	if(xpathObj == NULL) {
		char 	host_name[255];
		
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
		return CODE_ERR;
	}
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	
	if (size==1){
		if(nodes->nodeTab[0]->type == XML_ELEMENT_NODE)
			strncpy(value, xmlGetProp(nodes->nodeTab[0], "value"), size_value);
	}else{
		bzero(value, size_value);
	}
	xmlXPathFreeObject(xpathObj);
	return CODE_OK;
}


//-----------------------------------------------------------------------------
// 								lss_xml_get_options
//-----------------------------------------------------------------------------

int 
lss_xml_get_options(XML_SESSION *sid, const char *ldrID, const char *procID, const int start_element, char *value[], size_t size_value)
{
    char 				xpath_query[255];
	char				*str;
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size, i;
	int					j = 0;

    if (sid == NULL) return CODE_ERR;
	
	j = start_element;	
	
	sprintf(xpath_query, "/configuration/loader[@name='%s']/processor[@procID='%s']", ldrID, procID);

	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
	if(xpathObj == NULL) {
		char host_name[255];
	
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
        return CODE_ERR;
    }
	
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	xmlXPathFreeObject(xpathObj);
		
	if (size>0) {									
		sprintf(xpath_query, "/configuration/loader[@name='%s']/processor[@procID='%s']/param[@name='options']", ldrID, procID);

		xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
		if(xpathObj == NULL) {
			char host_name[255];

			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
			xmlXPathFreeObject(xpathObj);
        	return CODE_ERR;
    	}

    	nodes = xpathObj->nodesetval;
		size = (nodes) ? nodes->nodeNr : 0;
    
    	for(i = 0; i < size; ++i) {
			if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
				str = strdup(xmlGetProp(nodes->nodeTab[i], "value"));
				value[j] = str;
				if (j<size_value) j++;
			}
    	}

		xmlXPathFreeObject(xpathObj);
	}else {
		char host_name[255];

		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Loader Error: processor '%s' not found\n", host_name, getpid(), getppid(), procID);
		return CODE_ERR;
	}
	value[j] = NULL;
	
    return CODE_OK;
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_element
//-----------------------------------------------------------------------------
int
lss_xml_get_element(XML_SESSION *sid, const char *ldrID, const char *procID, const char *data, char *value, size_t size_value)
{
	char 				xpath_query[255];
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size;
	
	sprintf(xpath_query, "/configuration/loader[@name='%s']/processor[@procID='%s']/%s", ldrID, procID, data);

	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
	if(xpathObj == NULL) {
		char host_name[255];

		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
        return CODE_ERR;
    }

   	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	
	if (size==1){
		if(nodes->nodeTab[0]->type == XML_ELEMENT_NODE)
			strncpy(value, xmlNodeGetContent(nodes->nodeTab[0]), size_value);
	} else {
		char 	host_name[255];
		
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: processor procID='%s' return null result or multiline value\n", host_name, getpid(), getppid(), procID);
		xmlXPathFreeObject(xpathObj);
		return CODE_ERR;
	}
	xmlXPathFreeObject(xpathObj);
	return CODE_OK;
	
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_elements_open
//-----------------------------------------------------------------------------
XML_POOL		
*lss_xml_get_elements_open(XML_SESSION *sid, const char *ldrID, const char *procID, const char *data)
{
	XML_POOL 			*pool = NULL;
	char 				xpath_query[255];
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;

	if (sid == NULL) return NULL;
		
	if ((pool = (XML_POOL *) malloc(sizeof(XML_POOL)))==NULL) {
		char 	host_name[255];
		
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] error allocate memory for struct XML_POOL\n", host_name, getpid(), getppid());
		return NULL;
	}
	bzero(pool, sizeof(XML_POOL));
	
	pool->sid 		= sid;
	pool->xpathObj	= NULL;
	pool->nodes		= NULL;
	pool->ptr		= 0;
	pool->size		= 0;

	sprintf(xpath_query, "/configuration/loader[@name='%s']/processor[@procID='%s']/%s", ldrID, procID, data);

	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
	if(xpathObj == NULL) {
		char host_name[255];

		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
		free(pool);
        return NULL;
    }

   	nodes = xpathObj->nodesetval;
	pool->xpathObj 	= xpathObj;
	pool->nodes		= nodes;
	pool->size = (nodes) ? nodes->nodeNr : 0;
	return pool;
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_elements_fetch
//-----------------------------------------------------------------------------
char			
*lss_xml_get_elements_fetch(XML_POOL *pool)
{
	char	*res;
    xmlNodeSetPtr 		nodes = (xmlNodeSetPtr) pool->nodes;
		
	if (pool->ptr >= pool->size) return NULL;
		
	res = xmlNodeGetContent(nodes->nodeTab[pool->ptr]);
	pool->ptr++;	
	
	return res;
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_elements_close
//-----------------------------------------------------------------------------
int				
lss_xml_get_elements_close(XML_POOL *pool)
{
	xmlXPathFreeObject(pool->xpathObj);
	free(pool);
	pool = NULL;
                                                                                                                                                             
	return CODE_OK;
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_params_open
//-----------------------------------------------------------------------------
XML_POOL		
*lss_xml_get_params_open(XML_SESSION *sid, const char *ldrID, const char *procID, const char *param)
{
	XML_POOL 			*pool = NULL;
	char 				xpath_query[255];
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;

	if (sid == NULL) return NULL;
		
	if ((pool = (XML_POOL *) malloc(sizeof(XML_POOL)))==NULL) {
		char 	host_name[255];
		
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] error allocate memory for struct XML_POOL\n", host_name, getpid(), getppid());
		return NULL;
	}
	bzero(pool, sizeof(XML_POOL));
	
	pool->sid 		= sid;
	pool->xpathObj	= NULL;
	pool->nodes		= NULL;
	pool->ptr		= 0;
	pool->size		= 0;

	sprintf(xpath_query, "/configuration/loader[@name='%s']/processor[@procID='%s']/param[@name='%s']", ldrID, procID, param);

	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
	if(xpathObj == NULL) {
		char host_name[255];

		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
		free(pool);
        return NULL;
    }

   	nodes = xpathObj->nodesetval;
	pool->xpathObj 	= xpathObj;
	pool->nodes		= nodes;
	pool->size = (nodes) ? nodes->nodeNr : 0;
	return pool;	
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_params_fetch
//-----------------------------------------------------------------------------
char			
*lss_xml_get_params_fetch(XML_POOL *pool)
{
	char			*res;
    xmlNodeSetPtr 	nodes = (xmlNodeSetPtr) pool->nodes;
		
	if (pool->ptr >= pool->size) return NULL;
	
	res = xmlGetProp(nodes->nodeTab[pool->ptr], "value");
	pool->ptr++;	
	
	return res;	
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_params_fetch2
//-----------------------------------------------------------------------------
char			
*lss_mxl_get_params_fetch2(XML_POOL *pool, char *value, size_t size_value)
{
	char			*res;
    xmlNodeSetPtr 	nodes = (xmlNodeSetPtr) pool->nodes;
		
	if (pool->ptr >= pool->size) return NULL;
	
	res = xmlGetProp(nodes->nodeTab[pool->ptr], "value");
	pool->ptr++;	
	
	strncpy(value, res, size_value);
	return value;	
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_params_close
//-----------------------------------------------------------------------------
int				
lss_xml_get_params_close(XML_POOL *pool)
{
	xmlXPathFreeObject(pool->xpathObj);
	free(pool);
	pool = NULL;
                                                                                                                                                             
	return CODE_OK;
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_value
//-----------------------------------------------------------------------------
int 
lss_xml_get_value(XML_SESSION *sid, const char *xpath_query, char *value, size_t size_value) 
{
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size;
	
	if (sid == NULL) return CODE_ERR;
		
	bzero(value, size_value);
	
	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
	if(xpathObj == NULL) {
		char host_name[255];
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
        return CODE_ERR;
    }
	
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	if (size==1) {
		if(nodes->nodeTab[0]->type == XML_ELEMENT_NODE) {
			strncpy(value, xmlGetProp(nodes->nodeTab[0], "value"), size_value);
		}else{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: section '%s' no correct\n", host_name, getpid(), getppid(), xpath_query);
			xmlXPathFreeObject(xpathObj);
        	return CODE_ERR;
		}
	}else{
		char host_name[255];
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: query '%s' return null or multiline result\n", host_name, getpid(), getppid(), xpath_query);
		xmlXPathFreeObject(xpathObj);
       	return CODE_ERR;
	}
	return CODE_OK;
}

//-----------------------------------------------------------------------------
// 								lss_xml_get_attr
//-----------------------------------------------------------------------------
int 
lss_xml_get_attr(XML_SESSION *sid, const char *xpath_query, const char *attr, char *value, size_t size_value) 
{
    xmlXPathObjectPtr 	xpathObj; 
    xmlNodeSetPtr 		nodes;
    int 				size;
	
	bzero(value, size_value);
	
	xpathObj = xmlXPathEvalExpression(xpath_query, sid->xpathCtx);
    
	if(xpathObj == NULL) {
		char host_name[255];
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: Unable to evaluate xpath expression '%s'\n", host_name, getpid(), getppid(), xpath_query);
        return CODE_ERR;
    }
	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	if (size==1) {
		if(nodes->nodeTab[0]->type == XML_ELEMENT_NODE) {
			if (xmlGetProp(nodes->nodeTab[0], attr)!=NULL)
				strncpy(value, xmlGetProp(nodes->nodeTab[0], attr), size_value);
		}else{
			char host_name[255];
			gethostname(host_name, 255);
			fprintf(stderr,"[%s][%d:%d] Error: section '%s' no correct\n", host_name, getpid(), getppid(), xpath_query);
			xmlXPathFreeObject(xpathObj);
        	return CODE_ERR;
		}
	}else{
		char host_name[255];
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Error: query '%s' return null or multiline result\n", host_name, getpid(), getppid(), xpath_query);
		xmlXPathFreeObject(xpathObj);
       	return CODE_ERR;
	}
	return CODE_OK;
}
