/* Author: Alexander Hughey
 * CS4760 F2020
 * Project 5 - Resource Management
 * main driver for project 5
 * invoked as "oss"
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/msg.h> 
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

//sighandler attempt
//void sighandler(int);

struct mesg_buffer 
{ 
  long mesg_type; 
  char *mesg_text; 
}; 

int main(int argc, char* argv[])
{
  //variable declarations for command line options and defaults
  int opt;
  srand(time(0));
  const int maxTimeBetweenNewProcsSecs = 2;
  const int maxTimeBetweenNewProcsNS = 1000000;
  printf("rand NS=%d rand S=%d\n",maxTimeBetweenNewProcsNS,maxTimeBetweenNewProcsSecs);
  char * log_file = "logfile.txt";
  
  //////////BEGIN COMMAND LINE PARSING//////////
  //options are -h, -c x, -l file, -t time
  while((opt = getopt(argc, argv, ":hc:l:t:")) != -1)
  {
    switch(opt)
    {
      case 'h':
        printf("Usage:\n");
        printf("./oss\n");
        break;
      case'?':
        printf("unknown option\n");
        perror("ERROR: ");
        return(-1);
        break;
    }//end of switch
  }//end of while loop
  
  //catches extra arguments
  for(; optind < argc; optind++)
  { 
    char * test_str_file = "bb.bb";
    test_str_file = argv[optind];
    printf("extra argument: %s\n", test_str_file);  
  }//end of for loop
  //////////END COMMAND LINE PARSING//////////
  
  // clearing/creating log file
  FILE *fp1; 
  fp1 = fopen(log_file, "w");
  fprintf(fp1, "");
  fclose(fp1);
  
  //creating structure for process control block
  struct resource
  {
    int time_cpu;
    int time_tot;
    int time_last;
    int sim_PID;
    int priority;
    //todo
  };
  
  struct pcb pct[18];
  
  // create shared memory for clock.
  // seconds, nanosecond, pcb
  //keys for shmget
  key_t sec_key; 
  key_t ns_key;
  key_t pct_key;
  //ids returned from shmget
  int sec_shmid;
  int ns_shmid;
  int pct_shmid;
  //shared mem size (same for all 3)
  int size_clock;
  int size_table;
  
  //inserting values
  sec_key = 909091;
  ns_key = 808081;
  pct_key = 707071;
  size_clock = sizeof(int);
  size_table = (sizeof(struct pcb));
  printf("size_table = %d\n", size_table);
  
  //creating shared memory for seconds, nanosecond, process control table
  if((sec_shmid = shmget( sec_key, size_clock, 0666 | IPC_CREAT)) == -1)
  {
    perror("failed to create shared memory for clock seconds");
    return -1;
  }//end of if
  if((ns_shmid = shmget( ns_key, size_clock, 0666 | IPC_CREAT)) == -1)
  {
    perror("failed to create shared memory for clock nanoseconds");
    //destroying previously allocated shared memory segments
    if((shmctl( sec_shmid, IPC_RMID, NULL)) == -1)
    {
      perror("failed to destroy shared memory for sec");
      return -1;
    }//end of if
    return -1;
  }//end of if
  if((pct_shmid = shmget( pct_key, size_table, 0666 | IPC_CREAT)) == -1)
  {
    perror("failed to create shared memory for proces control table");
    //destroying previously allocated shared memory segments
    if((shmctl( sec_shmid, IPC_RMID, NULL)) == -1)
    {
      perror("failed to destroy shared memory for sec");
      return -1;
    }//end of if
    if((shmctl( ns_shmid, IPC_RMID, NULL)) == -1)
    {
      perror("failed to destroy shared memory for nanosec");
      return -1;
    }//end of if
    return -1;
  }//end of if
  
  //////////////////////////////////////////////////////////////////////
  //////////////////////ATTACH//AND//DETACH//BLOCK//////////////////////
  //////////////////////////////////////////////////////////////////////
  int *shm_sec;
  int *shm_nan;
  struct pcb *shm_pct;
  
  if((shm_sec = shmat( sec_shmid, NULL, 0)) == (int *) -1)
  {
    perror("failed to attach shared memory for clock seconds");
    return -1;
  }//end of if
  if((shm_nan = shmat( ns_shmid, NULL, 0)) == (int *) -1)
  {
    perror("failed to attach shared memory for clock nanoseconds");
    return -1;
  }//end of if
  if((shm_pct = shmat( pct_shmid, NULL, 0)) == (struct pcb *) -1)
  {
    perror("failed to attach shared memory for pct");
    return -1;
  }//end of if
  
  //setting values
  *shm_sec = 0;
  *shm_nan = 0;
  *shm_pct = *pct;
  
  /*debug
  printf("1. shm sec in main = %d\n", *shm_sec);
  printf("1. shm nan in main = %d\n", *shm_nan);
  printf("1. shm PID in main = %d\n", *shm_PID);*/
  
  //////////////////////////////////////////////////////////////////////
  ////////////////////////////END//OF//BLOCK////////////////////////////
  //////////////////////////////////////////////////////////////////////
  
  //////////logical loop begin//////////
  //creating and initalizing bit vector
  int proc_bit[18];
  int i = 0;
  for(i = 0; i < 18; i++) proc_bit[i] = 0;
  //array of character arrays used as an argument for exec
  /*debug
  printf("sending values\nsec_shmid:%d\nns_shmid:%d\n", sec_shmid, ns_shmid);
  printf("PID_shmid:%d\n", shmPID_shmid);*/
  char sec_id_str[50];  
  char nan_id_str[50];
  char pct_id_str[50];
  sprintf(sec_id_str, "%d", sec_shmid);  
  sprintf(nan_id_str, "%d", ns_shmid); 
  sprintf(pct_id_str, "%d", pct_shmid);
  char *args[]={"./user_proc", sec_id_str, nan_id_str, pct_id_str,NULL};
  //other variables for loop
  int flag_end = 0;
  int process_total = 0;
  int randNS = (rand()%maxTimeBetweenNewProcsNS);
  int randS = (rand()%maxTimeBetweenNewProcsSecs);
  int time_since_last = 0;
  int nano = 1000000000;
  int pid_loc = -1;
  //create message queue
  key_t msg_key; 
  int msg_id; 
  struct mesg_buffer message;
  msg_key = ftok("alexmsg", 65); 
  msg_id = msgget(msg_key, 0666 | IPC_CREAT); 
  //creating priority queues
  //1 is highest priority. 3 is lowest.
  struct pcb prio1[18];
  struct pcb prio2[18];
  struct pcb prio3[18];
  
  
  while (flag_end == 0)
  {
    if(time_since_last>=(maxTimeBetweenNewProcsSecs*nano)+maxTimeBetweenNewProcsNS)
    {
      pid_loc = -1;
      for(i = 0; i < 18; i++)
      {
        if(proc_bit[i] == 0) pid_loc = i;
      }
      if(pid_loc != -1)
      {
        proc_bit[pid_loc] = 1;
        //writing to log file
        FILE *fp1; 
        fp1 = fopen(log_file, "a");
        fprintf(fp1, "OSS: Generating process with PID %d", pid_loc);
        fprintf(fp1, " and putting it in queue 1 ");
        fprintf(fp1, "at time %d:%d\n", *shm_sec, *shm_nan);
        fclose(fp1);
        //creating pcb
        struct pcb temp;
        temp.time_cpu = 0;
        temp.time_tot = 0;
        temp.time_last = 0;
        temp.sim_PID = pid_loc;
        temp.priority = 1;
        
        //enter critical section
        shm_pct[pid_loc] = temp;
        //leave critical section
        
        time_since_last = 0;
        process_total++;
        
        randNS = (rand()%maxTimeBetweenNewProcsNS);
        randS = (rand()%maxTimeBetweenNewProcsSecs);
        
        if(fork() == 0)//child enter
        { 
          execvp(args[0],args);
          return 0;
        }//end of if
      }
    } 
    
    //scheduling process
    *shm_nan = *shm_nan + 8000;
    if(*shm_nan >= nano)
    {
      *shm_nan = *shm_nan%nano;
      *shm_sec = *shm_sec+1;
    }
    int flag_sched = 0;
    for(i = 0; i < 18; i++)
    {
      if(prio1[i].sim_PID != -1)
      {
        flag_sched = 1;
        message.mesg_type = prio1[i].sim_PID;  
        message.mesg_text = "800000";
        msgsnd(msg_id, &message, sizeof(message), 0);
        //writing to log file
        FILE *fp1; 
        fp1 = fopen(log_file, "a");
        fprintf(fp1, "OSS: Dispatching process with PID %d", prio1[i].sim_PID);
        fprintf(fp1, " from queue 1 ");
        fprintf(fp1, "at time %d:%d\n", *shm_sec, *shm_nan);
        fclose(fp1);
      }
      if (flag_sched == 1) break;
    }
    if (flag_sched == 0)
    {
      for(i = 0; i < 18; i++)
      {
        if(prio2[i].sim_PID != -1)
        {
          flag_sched = 1;
          message.mesg_type = prio2[i].sim_PID;  
          message.mesg_text = "800000";
          msgsnd(msg_id, &message, sizeof(message), 0);
          //writing to log file
          FILE *fp1; 
          fp1 = fopen(log_file, "a");
          fprintf(fp1, "OSS: Dispatching process with PID %d", prio2[i].sim_PID);
          fprintf(fp1, " from queue 2 ");
          fprintf(fp1, "at time %d:%d\n", *shm_sec, *shm_nan);
          fclose(fp1);
        }
        if (flag_sched == 1) break;
      }
    }
    if (flag_sched == 0)
    {
      for(i = 0; i < 18; i++)
      {
        if(prio3[i].sim_PID != -1)
        {
          flag_sched = 1;
          message.mesg_type = prio3[i].sim_PID;  
          message.mesg_text = "800000";
          msgsnd(msg_id, &message, sizeof(message), 0);
          //writing to log file
          FILE *fp1; 
          fp1 = fopen(log_file, "a");
          fprintf(fp1, "OSS: Dispatching process with PID %d", prio3[i].sim_PID);
          fprintf(fp1, " from queue 3 ");
          fprintf(fp1, "at time %d:%d\n", *shm_sec, *shm_nan);
          fclose(fp1);
        }
        if (flag_sched == 1) break;
      }
    }
    
    //flag_end = 1;
    if(process_total >= 100) flag_end = 1;
    time_since_last = time_since_last + 1000000;
    //enter critical section
    *shm_nan = *shm_nan + 1000000;
    if(*shm_nan >= nano)
    {
      *shm_nan = *shm_nan%nano;
      *shm_sec = *shm_sec+1;
    }
    //leave critical section
  }
  wait(NULL);
  //////////logical loop end//////////
  
  //detaching shared memory segments
  if((shmdt(shm_sec)) == -1)
  {
    perror("failed to detach shared memory for clock seconds");
    return -1;
  }//end of if
  if((shmdt(shm_nan)) == -1)
  {
    perror("failed to detach shared memory for clock nanoseconds");
    return -1;
  }//end of if
  if((shmdt(shm_pct)) == -1)
  {
    perror("failed to detach shared memory for pct");
    return -1;
  }//end of if
  
  //////////destroying shared memory segments//////////
  if((shmctl( sec_shmid, IPC_RMID, NULL)) == -1)
  {
    perror("failed to destroy shared memory for sec");
    return -1;
  }//end of if
  if((shmctl( ns_shmid, IPC_RMID, NULL)) == -1)
  {
    perror("failed to destroy shared memory for nanosec");
    return -1;
  }//end of if
  if((shmctl( pct_shmid, IPC_RMID, NULL)) == -1)
  {
    perror("failed to destroy shared memory for table");
    return -1;
  }//end of if
  
  return 0;
}//end of main