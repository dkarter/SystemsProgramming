/* 
 * Tsh - A tiny shell program with job control
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/* some user functions */
pid_t Fork();
void Sigaddset(sigset_t *set, int signo);
void Sigemptyset(sigset_t *set);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oset);
void Kill(pid_t pid, int signo);
int Setpgid(pid_t pid, pid_t pgid);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */
    
    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);
    
    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
            case 'h':             /* print help message */
                usage();
                break;
            case 'v':             /* emit additional diagnostic info */
                verbose = 1;
                break;
            case 'p':             /* don't print a prompt */
                emit_prompt = 0;  /* handy for automatic testing */
                break;
            default:
                usage();
        }
    }
    
    /* Install the signal handlers */
    
    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
    
    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 
    
    /* Initialize the job list */
    initjobs(jobs);
    
    /* Execute the shell's read/eval loop */
    while (1) {
        
        /* Read command line */
        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }
        
        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);
        fflush(stdout);
    } 
    
    exit(0); /* control never reaches here */
}


void Sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    if (sigprocmask(how, set, oset) < 0) //attempt block
        unix_error("Blocking signal error"); //report if failed
    
    return;
}


void Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0) //attempt set empty
        unix_error("Initializing empty signal set error"); //report if failed
    
    return;
}


void Sigaddset(sigset_t *set, int signo)
{
    if (sigaddset(set, signo) < 0) //attempt set add
        unix_error("Adding a signal to signal set error"); //report if failed
    
    return;
}


pid_t Fork()
{
    pid_t pid;
    if ((pid = fork()) < 0)
        unix_error("fork error");
    return pid;
}

int Setpgid(pid_t pid, pid_t pgid)
{
    int result;
    if ((result = setpgid(pid, pgid)) < 0)
        unix_error("setpgid error");
    return result;
}

/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
 */
void eval(char *cmdline) 
{
    pid_t pid;
    char *argv[MAXLINE];
    int bg = parseline(cmdline, argv);
    sigset_t sigmask;
    
    //check if built in commnd
    if (!builtin_cmd(argv)) {
        
        //block SIGCHLD signals to avoid conflicts when deleting a job
        Sigemptyset(&sigmask); //init empty signal set
        Sigaddset(&sigmask, SIGCHLD); //add the sigchld to the set
        
        Sigprocmask(SIG_BLOCK, &sigmask, NULL);
        
        
        //fork new process (gets pid at parent)
        if ((pid = Fork()) == 0) {
            //set process group id
            Setpgid(0,0);
            //unblock SIGCHLD on child
            Sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
            
            //if failed execution than report invalid command
            if (execve(argv[0], argv, environ) < 0) {
                printf("%s: Command not found\n", argv[0]);
                exit(1);
            }
            
        }
        
        addjob(jobs, pid, bg?BG:FG, cmdline);
        
        //unblock sigchld from parent
        Sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
        
        //handle background/foreground jobs
        if (!bg)
            waitfg(pid); //wait for forground job to stop or terminate
        else if (bg && pid != 0)
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline); //kust notify, dont wait
    }
    return;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */
    
    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
        buf++;
    
    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
        buf++;
        delim = strchr(buf, '\'');
    }
    else {
        delim = strchr(buf, ' ');
    }
    
    while (delim) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
            buf++;
        
        if (*buf == '\'') {
            buf++;
            delim = strchr(buf, '\'');
        }
        else {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
		return 1;
    
    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
        argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
    if (argv[0] == NULL)
        return 1;
    else if (strcmp(argv[0], "quit") == 0) // quit command
        exit(0);
    else if (strcmp(argv[0], "jobs") == 0) { //display jobs
        listjobs(jobs);
        return 1;
    }
    else if (strcmp(argv[0], "fg") == 0 || strcmp(argv[0], "bg") == 0) {
        do_bgfg(argv);
        return 1;
    }
    else if (strcmp(argv[0], "&") == 0) //ignore singleton
        return 1;
    
    return 0;     /* not a builtin command */
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
    // send SIGCONT to process on argv (pid/jid)
    //first we need to realize if bg or fg
    //then we need to figure out if we were supplied with 
    //a jid or a pid
    //use identifier  to get job obj
    //send signal SIGCONT to pid (obtained from job obj if we have jid)
    //change the status to BG or FG in job obj
    struct job_t *job;
    int jid;
    pid_t pid;
    int bg = !strcmp(argv[0], "bg");
    
    //if it starts with a % character than it's a jid
    if (argv[1] != NULL && argv[1][0] == '%') {
        //we have a jid! yay!
        //extract jid first (convert to int by removing % and using atoi)
        jid = atoi(argv[1] + sizeof(char));
        
        //check if numeric and valid
        if(jid == 0 || (job = getjobjid(jobs, jid)) == NULL) { 
            //not numeric or job doesn't exist
            printf("%s: No such job\n", argv[1]);
            return;
        } 
    } else if (argv[1] == NULL || (pid = atoi(argv[1])) == 0) {
        //not a pid or a jid - notify user.
        printf("%s: argument must be a PID or %%jobid\n", argv[1]);
        return;
    } else if ((job = getjobpid(jobs, pid)) == NULL) {
        //process id was invalid or not found in jobs list
        printf("(%s): No such process\n", argv[1]);
        return;
    }
    
    //so now we have the job let's do some magic
    //first try to send a SIGCONT
    Kill((job->pid), SIGCONT);
    
    //modify job object accordingly
    if (bg) {
        // change job state to background and just let it run
        job->state = BG;
        printf("[%d] (%d) %s\n", job->jid, job->pid, job->cmdline);
    } else {
        //change state to foreground
        job->state = FG;
        //since job is now in foreground we need to wait for it.
        waitfg(job->pid);
    }
    
    return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
    //first we have to check if the job exists so we don't waste our time
    if (getjobpid(jobs, pid) == NULL)
        return; //job not found
    
    //int i;
    while (fgpid(jobs) == pid)
        //call multiple times to make sure scheduler catches the sleep
        // for (i = 0; i < 5; i++)
        sleep(1); //context switches
    
    //don't forget to return
    return;
}

/*****************
 *Signal handlers
 *****************/

void Kill(pid_t pid, int signo) 
{
    //check if job exists or signal is failing to send
    if (getjobpid(jobs, pid) != NULL && kill(-pid, signo) < 0)
        unix_error("kill error");
    return;
}

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     Currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    
    int stat;
    pid_t pid;
    struct job_t *job; 
    
    while ((pid = waitpid(-1, &stat, WNOHANG | WUNTRACED)) > 0)
        if (WIFEXITED(stat))
            deletejob(jobs, pid);
        else if (WIFSTOPPED(stat)) {
            //change job status in list to stoppped
            job = getjobpid(jobs, fgpid(jobs));
            job->state = ST;
            
            printf("\nJob [%d] (%d) stopped by signal %d\n", job->jid, job->pid, WSTOPSIG(stat));
        }
        else if (WIFSIGNALED(stat)) {
            //get signal # of terminating signal
            //print first cause we  want the pid2jid to work and we can avoid storing it
            printf("\nJob [%d] (%d) terminated by signal %d\n", pid2jid(pid), pid, WTERMSIG(stat));
            //delete job - terminated by signal
            deletejob(jobs, pid);
        } 
        else
            unix_error("waitpid error");
    
    
    //if an error occured with the waitpid and it's not "no more children to reap" 
    //then report it to user
    if (!((pid == 0) || (pid == -1 && errno == ECHILD)))
        unix_error("waitpid error");
    
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    
    pid_t pid;
    if ((pid = fgpid(jobs)) > 0)
        Kill(pid, SIGINT); //forward to child
    
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    
    
    // first check who was it sent to, if there's
    // an acive foregroud proc we can assume that
    // it was the target otherwise it was sent to
    // the shell itself and we shall ignore it
    pid_t pid;
    if ((pid = fgpid(jobs)) > 0)
        //forward signal
        Kill(pid, SIGTSTP);
    
    return;  
    //return to tsh prompt
    
}


/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;
    
    for (i = 0; i < MAXJOBS; i++)
        clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;
    
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid > max)
            max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
        return 0;
    
    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
                nextjid = 1;
            strcpy(jobs[i].cmdline, cmdline);
            if(verbose){
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;
    
    if (pid < 1)
        return 0;
    
    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs)+1;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;
    
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].state == FG)
            return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;
    
    if (pid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid)
            return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;
    
    if (jid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid == jid)
            return &jobs[i];
    return NULL;
}


/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;
    
    if (pid < 1)
        return 0;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].state) {
                case BG: 
                    printf("Running ");
                    break;
                case FG: 
                    printf("Foreground ");
                    break;
                case ST: 
                    printf("Stopped ");
                    break;
                default:
                    printf("listjobs: Internal error: job[%d].state=%d ", 
                           i, jobs[i].state);
            }
            printf("%s", jobs[i].cmdline);
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;
    
    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */
    
    if (sigaction(signum, &action, &old_action) < 0)
        unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



