//this file has all of the functions and structures I used
//modifications were made upon provided queue.tar functions to be more compatible with job struct
#include <pthread.h>
#include <time.h>

void createarray(char *buf, char **array) {
	int i, count, len;
	len = strlen(buf);
	buf[len-1] = '\0'; /* replace last character (\n) with \0 */
	for (i = 0, array[0] = &buf[0], count = 1; i < len; i++) {
		if (buf[i] == ' ') {
		   buf[i] = '\0';
		   array[count++] = &buf[i+1];
		}
	}
	array[count] = (char *)NULL;
}
int openLog(char *fn)
{
    int fd;
    if ((fd = open(fn, O_CREAT | O_APPEND | O_WRONLY, 0755)) == -1)
    {
        fprintf(stderr, "Error: failed to open \"%s\"\n", fn);
        perror("open");
        exit(EXIT_FAILURE);
    }
    return fd;
}
typedef struct job
{
    int jobid, exitstat;     
    pthread_t tid; 
    char *command, *stat, *start, *stop, fout[20], ferr[20];            
} job;

job createjob(char *cmd, int jobid)
{
    job j;
    j.jobid = jobid;
    j.command = cmd;
    j.stat = "waiting";
    j.exitstat = -1;
    j.start = NULL;
    j.stop = NULL;
    sprintf(j.fout, "%d.out", j.jobid);
    sprintf(j.ferr, "%d.err", j.jobid);
    return j;
}
void showjobs(job *jobs, int n){
    int i;
    if (jobs != NULL && n != 0){
        for (i = 0; i < n; ++i){
            if (strcmp(jobs[i].stat, "complete") != 0)
                printf("Job ID: %d Command: %s Status: %s\n", jobs[i].jobid, jobs[i].command, jobs[i].stat);
		}
    }
}
void submithistory(job *jobs, int n){
	int i;
	if (jobs != NULL && n != 0){
		for (i = 0; i < n; ++i){
			if (strcmp(jobs[i].stat, "complete") == 0){
				printf("Job ID: %d Thread ID: %ld  Command: %s Exit Status: %d Start Time: %s Stop Time: %s\n", jobs[i].jobid, jobs[i].tid, jobs[i].command,jobs[i].exitstat,jobs[i].start,jobs[i].stop);
			}
        }
    }
}
	

typedef struct _queue {
	int size;    /* maximum size of the queue */
	job **buffer; /* queue buffer */
	int start;   /* index to the start of the queue */
	int end;     /* index to the end of the queue */
	int count;   /* no. of elements in the queue */
} queue;


/* create the queue data structure and initialize it */
queue *queue_init(int n)
{
    queue *q = malloc(sizeof(queue));
    q->size = n;
    q->buffer = malloc(sizeof(job *) * n);
    q->start = 0;
    q->end = 0;
    q->count = 0;

    return q;
}

/* insert an item into the queue, update the pointers and count, and
   return the no. of items in the queue (-1 if queue is null or full) */
int queue_insert(queue *q, job *jp)
{
    if ((q == NULL) || (q->count == q->size)){
        return -1;
	}
    q->buffer[q->end % q->size] = jp;
    q->end = (q->end + 1) % q->size;
    ++q->count;

    return q->count;
}

/* delete an item from the queue, update the pointers and count, and 
   return the item deleted (-1 if queue is null or empty) */
job *queue_delete(queue *q)
{
    if ((q == NULL) || (q->count == 0)){
        return (job *)-1;
	}
    job *j = q->buffer[q->start];
    q->start = (q->start + 1) % q->size;
    --q->count;

    return j;
}


/* delete the queue data structure */
void queue_destroy(queue *q) {
	free(q->buffer);
	free(q);
}

char *copyUntilNewLine(char *s)
{
    int i, c;
    char *copy;
    i = -1;
    copy = malloc(sizeof(char) * strlen(s));
    while ((c = s[++i]) != '\0' && c != '\n'){
        copy[i] = c;
	}
    copy[i] = '\0';
    return copy;
}

char *getCurrentTime()
{
    time_t t = time(NULL);
    return copyUntilNewLine(ctime(&t));
}

char *getCopy(char *s)
{
    int i, c;
    char *copy;
    i = -1;
    copy = malloc(sizeof(char) * strlen(s));
    while ((c = s[++i]) != '\0'){
        copy[i] = c;
	}
    copy[i] = '\0';
    return copy;
}