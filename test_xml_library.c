#include <stdio.h>
#include "../include/lss_conf.h"

int
main()
{
    XML_SESSION	*sid;
    XML_POOL	*pool;
    char	buf[255];
    char 	*arg[20];
    char	*str;
    int		i;

//----------------------------------------------------------------------------------------------    
    printf("STEP 1: Init xml library ");    
    if ((sid = lss_xml_init("/opt/lss/demo.conf"))==NULL){
	printf("ERROR INIT XML\n");
	exit(1);
    }
    printf("done\n\n");

//----------------------------------------------------------------------------------------------
    printf("STEP 2: Use function processor_xml_get_param:\n");    

    if (lss_xml_get_param(sid, "test", "proc1", "program", buf, sizeof(buf)) == 0) {
	printf("ERROR GET PARAM\n");
	exit(1);
    }
    printf("Result [ldrID=test procID=proc1]: %s\n\n", buf);
    
    printf("STEP 3: Use function processor_xml_get_options:\n");
    if (lss_xml_get_options(sid, "test", "proc2", 1, arg, sizeof(arg)) == 0) {
	printf("ERROR GET PARAM\n");
	exit(1);
    }
    
    for(i=1;i<20;i++){
	if (arg[i] == NULL) break;
	printf("Result [ldrID=test procID=proc2][%d] = '%s'\n", i, arg[i]);
    }

    printf("\nSTEP 4: Use function processor_xml_get_elements:\n");
    if ((pool = lss_xml_get_elements_open(sid, "test", "proc3", "data"))==NULL) exit(1);
    
    while ((str = lss_xml_get_elements_fetch(pool))!=NULL) {
	printf("Result [ldrID=test procID=proc3]: %s\n", str);
    }
    lss_xml_get_elements_close(pool);

    printf("\nSTEP 5: Use function processor_xml_get_element:\n");    

    if (lss_xml_get_element(sid, "test", "proc3", "test", buf, sizeof(buf)) == 0) {
	printf("ERROR GET PARAM\n");
	exit(1);
    }
    printf("Result [ldrID=test procID=proc3 element=test]: %s\n\n", buf);

    printf("STEP 6: Use function processor_xml_get_params_xxxx:\n");
    if ((pool = lss_xml_get_params_open(sid, "test", "proc2", "options"))==NULL) exit(1);
    
    while ((str = lss_xml_get_params_fetch(pool))!=NULL) {
	printf("Result [ldrID=test procID=proc3]: %s\n", str);
    }
    lss_xml_get_params_close(pool);
    

    printf("\nSTEP LAST: Free xml library ");    
    lss_xml_free(sid);
    printf(" done\n");    
    exit(0);
}
