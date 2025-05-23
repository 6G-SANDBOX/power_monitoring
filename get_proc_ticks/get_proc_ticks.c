#include <stdio.h>
#include <sys/types.h>
#include <sys/acct.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define HASHPROC struct {int key; struct proc_info* value;} 

#define HASHSIZE 4096
#define CMDLINE_LEN 4096
#define FILENAME_LEN 2048

#define DEFAULT_INTERVAL_SECS 3
#define PERCENT_METRIC_FORMAT "%d|%d|%s|%d.%02d\n"
#define POWER_METRIC_FORMAT "%d|%d|%s|%d\n"

struct proc_info{
  int pid, tcpu_ini, tcpu_end, tcpu;
  char name[CMDLINE_LEN];
};

int get_proc_time(int pid){
  FILE* f;
  char filename[FILENAME_LEN];
  int ucpu=0,scpu=0;
  snprintf(filename, sizeof(filename),  "/proc/%d/stat", pid);

  if ((f = fopen(filename, "r"))){
    if (!fscanf(f, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d %d %*s", &ucpu, &scpu))
      exit(-1);
  }

  if ((fclose (f))){
    exit(-1);
  }

  return ucpu+scpu;
}

void set_proc_name(int pid, char* n){
  int f;
  char filename[FILENAME_LEN];
  int l=0;

  snprintf(filename, sizeof(filename), "/proc/%d/cmdline", pid);

  if ((f = open(filename, O_RDONLY)))
    l = read(f, n, CMDLINE_LEN);

  for (int i=0; i<l; i++)
    if (n[i]=='\0') n[i]=' ';

  if (l==0)
    snprintf(n, FILENAME_LEN, "Kernel proc [%d]", pid);

  if ((close (f)))
    exit(-1);

}

int load_cpu_ticks(HASHPROC *intmap ) {
  DIR* d;
  struct dirent* cdir;
  int pid;
  struct proc_info *aux;

  d = opendir("/proc");
  while ((cdir=readdir(d))>0){
    if (sscanf(cdir->d_name, "%d", &pid)){
      aux = malloc(sizeof(struct proc_info));
      aux->pid = pid;
      aux->tcpu_ini = get_proc_time(pid);
      hmput(intmap, pid, aux);
    }
  } 
  closedir(d);

  return 0;
}

int main(int argc, char* argv[]) {
  DIR* d;
  struct dirent* cdir;
  int pid;
  struct proc_info *aux;
  int total_clicks=0;
  HASHPROC *intmap = NULL;

  unsigned int seconds = DEFAULT_INTERVAL_SECS;
  unsigned int vm_id = 0, power = 0;
  int opt;

  while ((opt = getopt(argc, argv, "hs:m:p:")) != -1) {
    switch (opt){
      case 's': seconds = atoi(optarg);
              break;
      case 'm': vm_id = atoi(optarg);
              break;
      case 'p': power = atoi(optarg);
              break;
      case 'h':
      default: printf ("Usage: %s [shMp]\n", argv[0]);
               printf (" h: this help\n");
               printf (" s [numsecs]: seconds to compute the consumption\n");
               printf (" m [id_vm]: VM ID to show in the metrics\n");
               printf (" p [power]: VM total power consumption in uW\n");

               exit(1);
               break;
    }
  }

  cdir = malloc(sizeof(struct dirent));

  d = opendir("/proc");
  while ((cdir=readdir(d))>0){
    if (sscanf(cdir->d_name, "%d", &pid)){
      aux = malloc(sizeof(struct proc_info));
      aux->pid = pid;
      aux->tcpu_ini = get_proc_time(pid);
      hmput(intmap, pid, aux);
    }
  } 
  closedir(d);

  sleep(seconds);

  d = opendir("/proc");
  while ((cdir=readdir(d))>0){
    if (sscanf(cdir->d_name, "%d", &pid)){
      aux = NULL;
      aux = hmget(intmap, pid);
      if (!aux) {
        aux = malloc(sizeof(struct proc_info));
        aux->pid = pid;
        aux->tcpu_ini = 0;
        hmput(intmap, pid, aux);
      }
      aux->tcpu_end = get_proc_time(pid);
      aux->tcpu = abs(aux->tcpu_end - aux->tcpu_ini);
      total_clicks += aux->tcpu;
    }
  } 
  closedir(d);

  for (int i=0; i < hmlen(intmap); i++){
    aux = intmap[i].value;
    if (aux->tcpu){
      set_proc_name(aux->pid, aux->name);
      if (!power)
        printf(PERCENT_METRIC_FORMAT, vm_id, aux->pid, aux->name, (aux->tcpu*100)/(total_clicks), ((aux->tcpu*10000)/(total_clicks))%100);
      else
        printf(POWER_METRIC_FORMAT, vm_id, aux->pid, aux->name, aux->tcpu*(power/total_clicks));
    }
  }

  hmfree(intmap);

  return (0);
}
