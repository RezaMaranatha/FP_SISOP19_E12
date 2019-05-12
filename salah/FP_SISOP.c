#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

void extc_time();
void extc_cmd();
void convert_time();
void run_cmd();
void check_mod();
void *crontab(void *arg);
void repeat();
char temp[10];
char temp2[200];
int i=0;
int sum=0;
int modified=0;
pthread_t tid[100];

typedef struct con
{
    int clock[5];
    char cmd[50];
}conn;

conn config[100];

void extc_time()
{
        int flag=0,j=0;
        FILE* cron;
        cron = fopen("crontab.data","r");
        while(!feof(cron))
        {
            int *arg = malloc(sizeof(*arg));
            memset(config[sum].clock, 0, sizeof(config[sum].clock)*5);
            fgets(temp2,200,cron);
            while(temp2[i]!='\0' && flag!=5)
            {
                temp[i] = temp2[i];
                if(temp2[i]==' ')
                    flag++;
                  
                i++;
            }
            convert_time();
            extc_cmd();
            *arg = sum;
            pthread_create(&tid[sum],NULL,crontab,arg);
            sum++;
            i=0;
            j=0;
            flag=0;
        }
        fclose(cron);
        repeat();
    
}

void t_kill(){
  int z;
  for(z=0;z<=sum;z++){
    pthread_cancel(tid[sum]);
  }
}

void repeat()
{

    while(1)
    {
        check_mod();
            if(modified==1)
            {      
                sum=0;
                modified=0;
                //t_kill();     
                memset(config, 0, sizeof(config));
                break;
            }
            sleep(1);
    }
    extc_time();
}

void convert_time(){

    char nol = '0';
    int j=0,l=0;
    while(temp[j]!='\0')
    {
        if(temp[j]!='*' && temp[j]!=' ' && temp[j+1]!='\0')
        {
            if(temp[j]!='*' && temp[j+1]!=' ' && temp[j+1]!='*') //ini buat ngecek clock 2 digit
            {
                config[sum].clock[l] = (temp[j] - nol)*10 + (temp[j+1]-nol);
                l++;    
            }
            else if(temp[j+1]==' ' && temp[j-1]==' ') //ngecek clock 1 digit
            {
                config[sum].clock[l] = temp[j] - nol;
                l++;
            }
        }
        else if(temp[j]=='*'){
            config[sum].clock[l]=-1;
            l++;
        }
        //printf("%d\n",con[sum].clock[l]);
        j++;    
    }
}

void run_cmd(int sum){
    pid_t tid_1= fork();
    if(tid_1==0)
    {
        execlp("bash","bash","-c",config[sum].cmd,NULL);
    }
}

void extc_cmd(){
    int a=0;
    int b=i;
    char temps[300];
    while(temp2[b]!='\0')
    {
        temps[a] = temp2[b];
        a++;
        b++;
    }
    temps[a+1]='\0';
    strcpy(config[sum].cmd,temps);
}

void *crontab(void *arg){
  int temp = *((int *) arg);
  while(1)
  {
      time_t t = time(NULL);
      struct tm tm = *localtime(&t);
      if (modified==1)
          {break;}
      if((config[temp].clock[0]==tm.tm_min || config[temp].clock[0]==-1) && (config[temp].clock[1]==tm.tm_hour || config[temp].clock[1]==-1) && (config[temp].clock[3]==tm.tm_mon+1 || config[temp].clock[3]==-1) &&((config[temp].clock[2]==tm.tm_mday || config[temp].clock[2]==-1) || (config[temp].clock[4]==tm.tm_wday || config[temp].clock[4]==-1)))
      {
         run_cmd(temp);
          sleep(60);
      }
      sleep(1);
  }
}

void check_mod()
{

  char file[] = "crontab.data";
  char timestamp[100] = "";
  struct stat buf;
  time_t ltime;
  if (!stat(file, &buf))
  {
  strftime(timestamp, 100, "%d-%m-%Y %H:%M:%S", localtime( &buf.st_mtime));
  }
  time( &ltime );
  if(difftime(ltime ,buf.st_mtime ) <= 1)
  {
    modified=1;
    sleep(2);
  }
  else{
    modified=0;
  }
}

int main() {
  pid_t pid, sid;

  pid = fork();

  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  umask(0);

  sid = setsid();

  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  // if ((chdir("/")) < 0) {
  //   exit(EXIT_FAILURE);
  // }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  // while(1) {
  //   // main program here
  //   sleep(30);
  // }
  extc_time();

  exit(EXIT_SUCCESS);
}