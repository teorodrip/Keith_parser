/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   launcher.c                                                               */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2018/12/20 17:48:49 by Mateo                                    */
/*   Updated: 2018/12/21 16:29:55 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define NAME_MAX 100
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define FILE_PATH "./notifications"
#define REBOOT_FILE_NAME "reboot.log"
#define START_FILE_NAME "start.log"
#define END_FILE_NAME "end.log"
#define VM_NAME "windows"
#define REBOOT_COMMAND "VBoxManage controlvm " VM_NAME " reset soft"
#define POWEROFF_COMMAND "VBoxManage controlvm " VM_NAME " poweroff soft"
#define START_COMMAND "VBoxManage startvm " VM_NAME
#define END_COMMAND "rm -f " FILE_PATH "/*"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void *time_out(void *arg)
{
  (void)arg;
  sleep(60);
  system(REBOOT_COMMAND);
  printf("Some problem booting the machine, proceding to reboot\n");
  pthread_mutex_lock(&mutex);
  if (pthread_create((pthread_t *)arg, NULL, time_out, (pthread_t *)arg))
	{
	  printf("Fatal: Can not create thread\n");
	  exit(2);
	}
  pthread_mutex_unlock(&mutex);
  return (NULL);
}

static void manage_event(struct inotify_event *i, pthread_t *time_watcher)
{
  char *file_name;

  file_name = NULL;
  printf("    wd =%2d; ", i->wd);
  printf("mask = ");
  if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
  if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
  if (i->mask & IN_CREATE)        printf("IN_CREATE ");
  if (i->mask & IN_DELETE)        printf("IN_DELETE ");
  if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
  if (i->mask & IN_MODIFY)        printf("IN_MODIFY ");
  if (i->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF ");
  if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
  if (i->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
  if (i->mask & IN_OPEN)          printf("IN_OPEN ");
  if (i->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
  printf("\n");
  if (i->len > 0)
	{
	  file_name = i->name;
	  printf("        name = %s\n", file_name);
	  if (!strcmp(file_name, REBOOT_FILE_NAME))
		{
		  printf("Rebooting VM\n");
		  system(REBOOT_COMMAND);
		  if (pthread_create(time_watcher, NULL, time_out, time_watcher))
			{
			  printf("Fatal: Can not create thread\n");
			  exit(2);
			}
		}
	  else if (!strcmp(file_name, START_FILE_NAME))
		{
		  pthread_mutex_lock(&mutex);
		  pthread_cancel(*time_watcher);
		  pthread_mutex_unlock(&mutex);
		  printf("Machine started successfully\n");
		}
	  else if (!strcmp(file_name, END_FILE_NAME))
		{
		  system(END_COMMAND);
		  system(POWEROFF_COMMAND);
		  printf("Data processed successfully\n");
		  exit(0);
		}
	}
}

int main()
{
  int		fd_notify;
  size_t	readed;
  char	buff[BUF_LEN];
  char	*buff_tmp;
  pthread_t time_watcher;
  struct inotify_event	*event;

  system(START_COMMAND);
  if (pthread_create(&time_watcher, NULL, time_out, &time_watcher))
	{
	  printf("Fatal: Can not create thread\n");
	  exit(2);
	}
  if ((fd_notify = inotify_init()) == -1)
  	{
  	  printf("Can not init inotify");
  	  exit(2);
  	}
  if (inotify_add_watch(fd_notify, FILE_PATH, IN_CLOSE_WRITE) == -1)
  	{
  	  printf("Can not add watcher");
  	  exit(2);
  	}
  while (1)
  	{
  	  if ((readed = read(fd_notify, buff, BUF_LEN)) < 0)
  		{
  		  printf("Error reading");
  		  exit(2);
  		}
  	  for (buff_tmp = buff; buff_tmp < buff + readed; )
  		{
  		  event = (struct inotify_event *)buff_tmp;
  		  manage_event(event, &time_watcher);
  		  buff_tmp += sizeof(struct inotify_event) + event->len;
  		}
  	}
  return (0);
}
