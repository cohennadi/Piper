#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>


int SUCCESS = 0;
int FAILED = -1;

void execute_command(char* command)
{
	int i = 0;
    int argument_count = 0;    
    const char SPACE = ' ';
	char *argvp[2] = {NULL, NULL};
	
    while (command[i] == SPACE) 
    {
        i++;
    }
    
    command = command + i;
    
    i = 0;
        
    while (command[i] != '\0') 
    {
         if (command[i] == SPACE)
         {
             argument_count++;
         }

         i++;
     }
     
    char** argv = calloc(argument_count + 2, sizeof(char*));
    char* argument = NULL;
    i = 0;        
    while ((argument = strsep(&command, " ")) != NULL) 
    {
       if (strlen(argument) != 0) 
       {
           argv[i] = calloc(strlen(argument) + 1, sizeof(char));
           strncpy(argv[i], argument, strlen(argument));
       }

       i++; 
    }

    argv[i] = NULL;
    argvp[0] = argv[0];        
    if (execvp(argv[0], argvp) != 0) {
        fprintf(stderr, "execvp failed. %s", strerror(errno));
    }
}

int wait_for_children(pid_t *children, int number_of_children)
{
	int status = 0;
	int i = 0;

	for (i = 0; i < number_of_children; ++i) 
	{    
        if (waitpid(children[i], &status, 0) != children[i]) 
        {
            return FAILED;
        }
    }

    return SUCCESS;
}

int piper(int argc, char *argv[])
{
	int returned_value = SUCCESS;
    int std_in = STDIN_FILENO;
    int std_out;
    int i; 
    int pipe_fd[2];
    pid_t pid;

	pid_t *children = calloc(argc, sizeof(pid_t));
    if (!children) 
    {
    	printf("calloc children failed");

        return FAILED;
    }

    for (i = 0; i < argc; ++i) 
    {
    	// Last
        if (i == argc - 1) 
        {   
        	pipe_fd[0] = -1;
            std_out = STDOUT_FILENO;         
            
        } 
        else 
        {
            if (pipe(pipe_fd) == FAILED) 
            {
                returned_value = FAILED;
            	goto exit;
            }

            std_out = pipe_fd[1];
        }

        pid = fork();
        if (pid == FAILED) 
        {
            returned_value = FAILED;
            goto exit;
        }

        // Child is running
        if (pid == 0)
         { 
            if (dup2(std_out, STDOUT_FILENO) == FAILED) 
            {
                return FAILED;
            }

            if (dup2(std_in, STDIN_FILENO) == FAILED)
            {
                return FAILED;
            }

            execute_command(argv[i]);
        }

        close(std_out);
        close(std_in);

        children[i] = pid;
        std_in = pipe_fd[0];
    }

    returned_value = wait_for_children(children, argc);

 exit:   
    free(children);

    return returned_value;
}


int main(int argc, char *argv[]) 
{	
	int returned_value = SUCCESS; 

	if (argc < 2)
	{
		printf("usage: piper _command1_ _command12_ ...");

		return FAILED;
	}

	if (argc == 2) 
	{
        execute_command(argv[1]);
    }

    return piper(argc, argv);
}