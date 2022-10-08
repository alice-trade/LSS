
#ifndef _LIB_XML_H
#define _LIB_XML_H 1

typedef void	*_xmlDocPtr;
typedef void	*_xmlXPathContextPtr;
typedef void	*_xmlXPathObjectPtr;
typedef void	*_xmlNodeSetPtr;

typedef struct st_xml_session
{
	char					*config_file;
	_xmlDocPtr 				doc;
    _xmlXPathContextPtr 	xpathCtx;
} XML_SESSION;

typedef struct st_xml_str_node
{
	char					*str;
	struct st_xml_str_node	*next;
	struct st_xml_str_node	*pred;		
} XML_STR_NODE;

typedef struct st_xml_pool
{
	XML_SESSION				*sid;

	_xmlXPathObjectPtr		xpathObj;
	_xmlNodeSetPtr 			nodes;
	int						ptr;
	int						size;
} XML_POOL;


XML_SESSION		*lss_xml_init(const char *config_file);
int				lss_xml_free(XML_SESSION *sid);
int				lss_xml_get_param(XML_SESSION *sid, const char *ldrID, const char *procID, char const *param, char *value, size_t size_value);
int				lss_xml_get_param_or_null(XML_SESSION *sid, const char *ldrID, const char *procID, char const *param, char *value, size_t size_value);

/* Function get elements */
int				lss_xml_get_element(XML_SESSION *sid, const char *ldrID, const char *procID, const char *data, char *value, size_t size_value);

XML_POOL		*lss_xml_get_elements_open(XML_SESSION *sid, const char *ldrID, const char *procID, const char *data);
char			*lss_xml_get_elements_fetch(XML_POOL *pool);
int				lss_xml_get_elements_close(XML_POOL *pool);


int				lss_xml_get_options(XML_SESSION *sid, const char *ldrID, const char *procID, const int start_element, char *value[], size_t size_value);

XML_POOL		*lss_xml_get_params_open(XML_SESSION *sid, const char *ldrID, const char *procID, const char *param);
char			*lss_xml_get_params_fetch(XML_POOL *pool);
char			*lss_mxl_get_params_fetch2(XML_POOL *pool, char *value, size_t size_value);
int				lss_xml_get_paramss_close(XML_POOL *pool);


int				lss_xml_get_value(XML_SESSION *sid, const char *xpath_query, char *value, size_t size_value);
int				lss_xml_get_attr(XML_SESSION *sid, const char *xpath_query, const char *attr, char *value, size_t size_value); 

#endif /* include/lib_xml.h */
