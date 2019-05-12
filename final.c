#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

int count_baru;
struct tm times;
struct tm old;
int flag=1;

struct conf
{
  int min;
  int hour;
  int day;
  int month;
  int day_week;
  char perintah[1001];
  time_t akses;
};

struct conf konfig[1001];

int check_wday(struct conf *myconf)
{
  int check=0;
  if(myconf->day_week == -1) check = 1;
  else if(myconf->day_week == times.tm_wday) check = 1;
  //printf("ini check %d\n", check);
  return check;
}

int check_month(struct conf *myconf)
{
  int check=0;
  if(myconf->month == -1) check = 1;
  else if(myconf->month == times.tm_mon) check = 1;
  return check_wday(myconf) & check;
}

int check_day(struct conf *myconf)
{
  int check=0;
  if(myconf->day == -1) check = 1;
  else if(myconf->day == times.tm_mday) check = 1;
  return check_month(myconf) & check;
}

int check_hour(struct conf *myconf)
{
  int check=0;
  if(myconf->hour == -1) check = 1;
  else if(myconf->hour == times.tm_hour) check = 1;
  int temp3 = check_day(myconf) & check;
  //printf("ini jam %d\n\n", temp3);
  return temp3;
}

int check_min(struct conf *myconf)
{
  int check=0;
  //printf("ini menit sistem: %d\n", times.tm_min);
  //printf("ini menit file: %d\n", myconf->min);
  if(myconf->min == -1) check = 1;
  else if(myconf->min == times.tm_min) check = 1;
  int temp1 = check_hour(myconf) & check;
  //printf("ini menit %d\n\n", temp1);
  return temp1;
}

void *check_config(void *arg)
{
  time_t crontim = time(NULL);
  int tanda = 0;
  struct conf *myconf = (struct conf *)arg;
  while(1)
  {
    time_t systim = time(NULL);
    
    if(flag==0) pthread_exit(NULL);
    
    int res = systim - crontim; 
    int abs = check_min(myconf);
    //printf("%d\n", abs);
    if(abs==1 && (res>=60 || tanda==0) ) 
    {
      pid_t child = fork();
      tanda = 1;
      crontim = systim;
      //system(myconf->perintah);
      //pthread_exit(NULL);
      if(child == 0) execl("/bin/sh", "sh", "-c", myconf->perintah, NULL);
    }
  }
}

void ambil_waktu()
{
  char temp[1001], car[5];
  int count = 0;
  memset(car,'\0',sizeof(car));
  int i=0;
  FILE* filenya;
  filenya = fopen("/home/idputuwiprah/FP/crontab.data","r");
  while(fgets(temp,sizeof(temp),filenya))
  {
    count++;
    int ptr=0, ptr2=0, tab=1;
    while(1)
    {
      if(temp[ptr]==' ')
      {
        if(tab==1)
        {

          if(strcmp(car,"*")==0) konfig[i].min = -1;
          else konfig[i].min = atoi(car);
          tab++;
          ptr2=0;
        }
        else if(tab==2)
        {
          if(strcmp(car,"*")==0) konfig[i].hour = -1;
          else konfig[i].hour = atoi(car);
          tab++;
          ptr2=0;
        }
        else if(tab==3)
        {
          if(strcmp(car,"*")==0) konfig[i].day = -1;
          else konfig[i].day = atoi(car);
          tab++;
          ptr2=0;
        }
        else if(tab==4)
        {
          if(strcmp(car,"*")==0) konfig[i].month = -1;
          else konfig[i].month = atoi(car);
          tab++;
          ptr2=0;
        }
        else if(tab==5)
        {
          if(strcmp(car,"*")==0) konfig[i].day_week = -1;
          else konfig[i].day_week = atoi(car);
          tab++;
          ptr2=0;
        }
        //car[ptr] = temp[ptr];
        memset(car,'\0',sizeof(car));
        ptr++;
      }
      else
      {
        car[ptr2] = temp[ptr];
        ptr++;
        ptr2++;
      }
      if(tab==6) break; 
    }
    count_baru = count;
    konfig[i].akses = 0;
    //printf("%d\n", ptr);
    strcpy(konfig[i].perintah,temp+ptr);
    //printf("waktu %d:%d:%d:%d:%d perintah: %s\n",konfig[i].min, konfig[i].hour, konfig[i].day, konfig[i].month, konfig[i].day_week, konfig[i].perintah);
    i++;
  }
  fclose(filenya);
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

  if ((chdir("/home/idputuwiprah/FP/")) < 0) {
    exit(EXIT_FAILURE);
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  count_baru = 0;
  
  time_t systime = time(NULL);
  times = *localtime(&systime);
  
  struct stat stt;
  stat("/home/idputuwiprah/FP/crontab.data", &stt);
  
  old = *localtime(&(stt.st_mtime));
  ambil_waktu();
  //printf("%d\n", count_baru);
  pthread_t tid[100001];
  for(int i=0; i<count_baru; i++)
  { 
    pthread_create(&tid[i], NULL, check_config, &konfig[i]);
  }

  while(1) {
    time_t systime = time(NULL);
    times = *localtime(&systime);

    memset(&stt, 0, sizeof(stt));
    stat("/home/idputuwiprah/FP/crontab.data", &stt);
    
    struct tm new = *localtime(&(stt.st_mtime));
    
    double seconds = difftime( mktime(&new), mktime(&old)); 

    if(seconds != 0.0)
    {
      flag=0;
      for(int i=0; i<count_baru; i++)
      {
        pthread_join(tid[i],NULL);
      }

      ambil_waktu();

      for(int i=0; i<count_baru; i++)
      { 
        flag=1;
        //printf("berubah cuk\n\n");
        //printf("%d\n", i);
        pthread_create(&tid[i], NULL, check_config, &konfig[i]);
      }

      old = *localtime(&(stt.st_mtime));
      //printf("\nsayang\n");
    }
    sleep(60);
  }
  
  exit(EXIT_SUCCESS);
}