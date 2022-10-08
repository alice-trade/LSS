/* 
 	Процессор для запуска удалённого лоадера
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <termios.h>

#include <config.h>
#include "../include/lss_misc.h"

static int get_name_loader(const char *str, char *value, size_t size_value);
static int start_loader(const char *ldrID, const char *t_date, const char *t_time, int *stream_in, int *stream_out);



int 
main(int argc, char **argv)
{
	char	buffer[1024];
	struct	termios settings;
	char	run_loader[255];
	int 	stream_in[2];
	int		stream_out[2];
	int		fd_in=-1, fd_out=-1;
	pid_t	pid = 0;

	DEFINE_ENV_PROCESSOR;
	
	if ( tcgetattr (STDOUT_FILENO, &settings) >= 0 ) {
            settings.c_lflag &= ~TOSTOP;
	    tcsetattr(STDOUT_FILENO,TCSANOW,&settings);
	}	
	
	if (check_argv_processor(argc, argv)!=CODE_OK) exit(1);
	
	while(!feof(stdin)){
		bzero(buffer, sizeof(buffer));
		
		fgets(buffer, sizeof(buffer), stdin);
		if (memcmp(buffer, "#LOADER", 7)==0){

// Проверяем, а не пишем ли мы уже куда нибудь?
			if (pid!=0){
				close(fd_out); // Закрываем df_in
				
				for(;;) {
					ssize_t size;
					char	buf[1024];
					
					bzero(buf, sizeof(buf));
		    		size = read(fd_in, buf, sizeof(buf));
		    		if (size==0) break;
		    		if (size==-1) CRIT_PROC_ERROR(procID, "read for pipe", errno);
					fprintf(stdout, "%s", buf);
				}
				close(fd_in);
				
				fprintf(stdout, "#END_BLOCK\n");

				for(;;){
					pid_t	t_pid;
					int		status;
					
					t_pid = waitpid(pid, &status, 0);
					if( t_pid == -1 && errno == EINTR ) continue;
					if (t_pid == -1) CRIT_PROC_ERROR(procID, "waitpid", 1)
					if (t_pid > 0) break;
		 		}
			}

// Начинаем обработку нового лоадера			
			get_name_loader(buffer, run_loader, sizeof(run_loader));			// Получаем имя лоадера

			if (pipe(stream_in)) CRIT_PROC_ERROR(procID, "create_pipe_in", errno);	// Создаём pipe
			if (pipe(stream_out)) CRIT_PROC_ERROR(procID, "create_pipe_out", errno);	// Создаём pipe

			fd_in	= stream_out[0];
			fd_out	= stream_in[1];

			pid = start_loader(run_loader, lss_date, lss_time, stream_in, stream_out);	// Запускаем лоадеры

			close(stream_out[1]);
			close(stream_in[0]);
			
			fprintf(stdout, "#BEGIN_BLOCK\n");									// Начинаем блок
		}else{
			if (pid!=0) write(fd_out, buffer, strlen(buffer));					// Промежуточные данные пишутся в открытый дескриптор
		}
	}

	if (pid!=0){
		close(fd_out); // Закрываем df_in
				
		for(;;) {
			ssize_t size;
			char	buf[1024];
			
			bzero(buf, sizeof(buf));			
    		size = read(fd_in, buf, sizeof(buf));
    		if (size==0) break;
    		if (size==-1) CRIT_PROC_ERROR(procID, "read for pipe", errno);
			fprintf(stdout, "%s", buf);
		}
		fprintf(stdout, "#END_BLOCK\n");
		close(fd_in);
	}

	exit(0);
}


static int
get_name_loader(const char *str, char *value, size_t size_value)
{
	char	result[255];
	int		cp;
	
	cp = sscanf(str, "#LOADER %s", result);
	
	if (cp<1) {
		return CODE_ERR;
	}
	strncpy(value, result, size_value);
	return CODE_OK;
}

static int 
start_loader(const char *ldrID, const char *t_date, const char *t_time, int *stream_in, int *stream_out)
{
	char	loader_path[FILENAME_MAX];
    pid_t pid = fork();
    
	if (pid == (pid_t) -1) {
		CRIT_PROC_ERROR(procID, "fork", errno);	/* ERROR */
    } else if (pid == 0) {    				/* CHILD */
		char host_name[255];

		signal(SIGTERM, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGINT,  SIG_DFL);
		signal(SIGALRM, SIG_DFL); 

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
	
		gethostname(host_name, 255);
		fprintf(stderr,"[%s][%d:%d] Loader Starting: (bin/lss_ldr %s %s %s)\n", host_name, getpid(), getppid(), ldrID, t_date, t_time);

		sprintf(loader_path, "%s/%s", LOADER_DIR, "lss_ldr");

		if (!t_time) {
			execl(loader_path, "lss_ldr", ldrID, t_date, NULL);
			CRIT_PROC_ERROR(procID, "execl with date", errno);
		} else {
			execl(loader_path, "lss_ldr", ldrID, t_date, t_time, NULL);
			CRIT_PROC_ERROR(procID, "execl with time", errno);
		}
    } else {    							/* PARENT */
		if(stream_in[0] != STDIN_FILENO) 
	    	close(stream_in[0]);
		if(stream_out[1] != STDOUT_FILENO) 
	    	close(stream_out[1]);
    }
    return pid;
}
