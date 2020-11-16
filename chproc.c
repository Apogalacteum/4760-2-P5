/* Author: Alexander Hughey
 * CS4760 F2020
 * Project 5 - Resource Management
 * User process for project 5
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
#include <pthread.h>
#include <signal.h>

//void sighandler(int);

int main(int argc, char *argv[])
{
  
  printf("In user\n");
  int sec_shmid = atoi(argv[1]);
  int ns_shmid = atoi(argv[2]);
  int pct_shmid = atoi(argv[3]);
  //signal(SIGINT, sighandler);
  /*debug
  printf("have values\nsec_shmid:%d\nns_shmid:%d\n", sec_shmid, ns_shmid);
  printf("PID_shmid:%d\n", shmPID_shmid);*/
  
  //generating random number based on process id
  //because I couldn't figure out how else to do it
  pid_t procid = getpid();
  int loopflag = 0;
  srand(procid);
  int lifetime = (rand()%1000000);
  //{critical section}
  //  start = time (ns + s*1000000000)
  //
  //while( loopflag == 0 )
  //    check clock
  //    if( current - start > lifetime )
  //      if( shmPID == 0 )
  //        shmPID = PID
  //        loopflag = 1
  //printf("test\tnum = %d\tseed = %d\n", lifetime, procid);
  
  
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //attaching and setting values for shared memory clock
  int *shm_sec;
  int *shm_nan;
  int *shm_PID;
  
  if((shm_sec = shmat( sec_shmid, NULL, 0)) == (int *) -1)
  {
    perror("failed to attach shared memory in user for clock seconds");
    return -1;
  }//end of if
  
  if((shm_nan = shmat( ns_shmid, NULL, 0)) == (int *) -1)
  {
    perror("failed to attach shared memory for clock nanoseconds");
    return -1;
  }//end of if
  
  if((shm_PID = shmat( shmPID_shmid, NULL, 0)) == (int *) -1)
  {
    perror("failed to attach shared memory for shmPID");
    return -1;
  }//end of if
  
  int start = (*shm_nan + (*shm_sec * 1000000000));
  
  /*debug
  *shm_sec = 30;
  *shm_nan = 40;
  *shm_PID = 50;
  
  printf("2. shm sec in user = %d\n", *shm_sec);
  printf("2. shm nan in user = %d\n", *shm_nan);
  printf("2. shm PID in user = %d\n", *shm_PID);*/
  
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
  
  if((shmdt(shm_PID)) == -1)
  {
    perror("failed to detach shared memory for shmPID");
    return -1;
  }//end of if
  
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  
  while( loopflag == 0 )
  {
    if((shm_sec = shmat( sec_shmid, NULL, 0)) == (int *) -1)
    {
      perror("failed to attach shared memory in user for clock seconds");
      return -1;
    }//end of if
    if((shm_nan = shmat( ns_shmid, NULL, 0)) == (int *) -1)
    {
      perror("failed to attach shared memory for clock nanoseconds");
      return -1;
    }//end of if
    if((shm_PID = shmat( shmPID_shmid, NULL, 0)) == (int *) -1)
    {
      perror("failed to attach shared memory for shmPID");
      return -1;
    }//end of if
    
    int current = (*shm_nan + (*shm_sec * 1000000000)); 
    if( current - start > lifetime )
    {
      if( *shm_PID == 0 )
      {
        *shm_PID = procid;
        loopflag = 1;
      }
    }
    
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
    
    if((shmdt(shm_PID)) == -1)
    {
      perror("failed to detach shared memory for shmPID");
      return -1;
    }//end of if
    
  }

  return 0;
}

/*
void sighandler(int signum) 
{
  printf("Caught signal %d, exiting...\n", signum);
  
  //////////////////detaching shared memory segments
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
  if((shmdt(shm_PID)) == -1)
  {
    perror("failed to detach shared memory for shmPID");
    return -1;
  }//end of if
  
  //////////////////destroying shared memory segments
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
  if((shmctl( shmPID_shmid, IPC_RMID, NULL)) == -1)
  {
    perror("failed to destroy shared memory for shmPID");
    return -1;
  }//end of if
  
  exit(1);
}*/