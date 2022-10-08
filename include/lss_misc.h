
#ifndef _MISC_H
#define _MISC_H 1

#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <stdarg.h>

#define CRIT_MANAGER_ERROR(name, func, code) { \
	char host_name[255];\
	gethostname(host_name, 255);\
    fprintf(stderr,"Critical Manager Error [%s][%d:%d][%s][%s]: %s\n", host_name, getpid(), getppid(), basename(name), func, strerror(errno));\
    exit(code); }
	
#define CRIT_LOADER_ERROR(name, func, code) { \
	char host_name[255];\
	gethostname(host_name, 255);\
    fprintf(stderr,"Critical Loader Error [%s][%d:%d][%s][%s]: %s\n", host_name, getpid(), getppid(), basename(name), func, strerror(errno));\
    terminate(SIGTERM); \
    exit(code); }

#define CRIT_PROC_ERROR(name, function, code) { \
        processor_print_error(name,"function %s: %s\n", function, strerror(code)); \
        exit(code); }


#define	CODE_OK				1
#define	CODE_ERR			0

	
#define	CONFIG_FILE			"/opt/lss/lss.conf"
#define LOADER_DIR			"/opt/lss/bin"

#define MAX_SIZE_MGRID		50
#define MAX_SIZE_LDRID		50
#define MAX_SIZE_PROCID		50
#define MAX_SIZE_CONFIG		FILENAME_MAX
#define MAX_SIZE_DATE		11
#define	MAX_SIZE_TIME		5

extern char *config_file;
extern char	*mgrID;
extern char *ldrID;
extern char *procID;
extern char *lss_date;
extern char *lss_time;

#define DEFINE_ENV_MANAGER { \
		mgrID = argv[1]; \
		if (argc > 2) lss_date = argv[2]; \
		if (argc > 3) lss_time = argv[3]; \
	}

#define DEFINE_ENV_LOADER { \
		ldrID = argv[1]; \
		if (argc > 2) lss_date = argv[2]; \
		if (argc > 3) lss_time = argv[3]; \
	}
	
#define DEFINE_ENV_PROCESSOR { \
		config_file = argv[1]; \
		ldrID		= argv[2]; \
		procID		= argv[3]; \
		lss_date	= argv[4]; \
		if (argc > 5) lss_time = argv[5]; \
	}

#define CLEAN_ENV_MANAGER { \
		bzero(mgrID, MAX_SIZE_MGRID); \
		bzero(m_date, MAX_SIZE_DATE); \
		bzero(m_time, MAX_SIZE_TIME); \
	}
	
#define CLEAN_ENV_LOADER { \
		bzero(ldrID, MAX_SIZE_LDRID); \
		bzero(l_date, MAX_SIZE_DATE); \
		bzero(l_time, MAX_SIZE_TIME); \
	}

#define CLEAN_ENV_PROCESSOR { \
		bzero(config_file, MAX_SIZE_CONFIG); \
		bzero(ldrID, MAX_SIZE_LDRID); \
		bzero(procID, MAX_SIZE_PROCID); \
		bzero(p_date, MAX_SIZE_DATE); \
		bzero(p_time, MAX_SIZE_TIME); \
	}
int		parse_argv_manager(int argc, char **argv, char *mgrID, char *t_date, char *t_time);
int 	parse_argv_loader(int argc, char **argv, char *ldrID, char *date, char *time);
int 	parse_argv_processor(int argc, char **argv, char *config_file, char *ldrID, char *date, char *time);
int		parse_argv_processor2(int argc, char **argv, char *config_file, char *ldrID, char *ProcID, char *date, char *time);

int		check_argv_manager(int argc, char **argv);
int 	check_argv_loader(int argc, char **argv);
int 	check_argv_processor(int argc, char **argv);
	
int		get_xml_value(const char *xpath_query, const char *config_file, char *value);
int 	load_option(const char *name_processor, const char *config_file, const char *ldrID, char *arg_v[]);
int 	load_option2(const char *name_processor, const char *config_file, const char *ldrID, char *arg_v[]);

void	processor_print_message(const char *proc, const char *fmt, ...);
void 	processor_print_error(const char *proc, const char *fmt, ...);
void	lss_print_error(const char *type_prog, const char *type_msg, const char *ID, const char *fmt, va_list ap);

#endif /* include/misc.h */
