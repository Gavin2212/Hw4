// to compile  gcc -pthread -o hw4 Hw4.c
// to execute ./hw4 'p' (if no p provided default of 2 will be used)
//commands: submit 'command', showjobs, submithistory, exit
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include "extra.h"

int p, numworking;
job jobs[100];
queue *q;

void *currentjob(void *arg){
    time_t t1, t2;
    job *jp;     
    pid_t pid;   
    char *args[100];
    jp = (job *)arg;
    ++numworking;
    jp->stat = "running";
    time(&t1);
    jp->start = getCurrentTime();
    pid = fork();
    if (pid == 0) //child process
    {
        dup2(openLog(jp->fout), STDOUT_FILENO); //route job to stdout
        dup2(openLog(jp->ferr), STDERR_FILENO); //reroute job to stderr
        createarray(jp->command, args);
        execvp(args[0], args);
        fprintf(stderr, "Error: command failed for \"%s\"\n", args[0]);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) // parent process
    {
        waitpid(pid, &jp->exitstat, WUNTRACED);
        jp->stat = "complete";
        time(&t2);
        jp->stop =getCurrentTime();
    } else{
        fprintf(stderr, "Error: process failed\n");
        perror("fork");
        exit(EXIT_FAILURE);
    }
    --numworking;
    return NULL;
}
void *handlecurrentjobs(void *arg)
{
    job *jp;
    numworking = 0;
    while(1) {
        if (q->count > 0 && numworking < p)
        {   
            jp = queue_delete(q);        //get next job in waiting queue and delete it from queue
            pthread_create(&jp->tid, NULL, currentjob, jp);       //create individual thread to handle the singular job running
            pthread_detach(jp->tid);            //detach at the end to free up resources
        }
        sleep(1); //wait a second then loop again
    }
    return NULL;
}


int main(int argc,char *argv[]){
    int  jobnum=0;   
    char command[50], *args[100];
    pthread_t tid;
    job jobs[100];

    if(argv[1] ==NULL){
        printf("no p value provided, default max queue set to 2 \n");
        p =2;
    }else{
        p = atoi(argv[1]);
    }
   // printf("p value: %i\n", p);

	q = queue_init(100);                //queue for waiting jobs

    pthread_create(&tid, NULL, handlecurrentjobs, NULL);     //create thread to handle current jobs

//------------------------------------------------------------------//all below handles user input
    while(1){
        printf("Enter command>");
        fgets(command, 100, stdin);
        char jcommand[100];
        //printf("%s",  command);
        createarray(command, args);
        /*for (int i = 0; args[i] != NULL; i++)
            printf("[%s] ", args[i]);
        printf("\n");*/
        //parse first command
        if (strcmp(args[0], "submit") == 0) {              
            for (int i = 1; args[i] != NULL; i++){
                if(i>1){
                    strcat(jcommand, " ");          //create job command by removing submit from initial command
                    strcat(jcommand, args[i]);
                }else{
                    strcpy(jcommand, args[i]);
                }
            }
            //printf("case: submit\n");
            jobs[jobnum] = createjob(getCopy(jcommand), jobnum);             //create job and insert its pointer into queue
            queue_insert(q, jobs+jobnum);
            printf("job %d added to the queue with command: %s\n", jobnum++, jobs[jobnum].command);
        

        }else if (strcmp(args[0], "showjobs") == 0) {
            //printf("case: showjobs\n");
            showjobs(jobs, jobnum);

        }else if (strcmp(args[0], "submithistory") == 0) {
            submithistory(jobs,jobnum);

        }else if (strcmp(args[0], "exit") == 0) {
            exit(-1);

        }else{
            printf("invalid command \n");
        }
    }
}