/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   inotify.c                                                                */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/02 13:51:12 by Mateo                                    */
/*   Updated: 2019/01/03 10:23:56 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/launcher.h"
#include <sys/stat.h>

int manage_event(struct inotify_event *i, time_out_data_t *tim_dat, char *poweroff)
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
					system(tim_dat->reboot);
					if (pthread_create(tim_dat->thread, NULL, time_out, (void *)tim_dat))
						{
							printf("Fatal: Can not create thread\n");
							exit(2);
						}
				}
			else if (!strcmp(file_name, START_FILE_NAME))
				{
					pthread_mutex_lock(mutex + tim_dat->id);
					start_success[tim_dat->id] = 1;
					pthread_mutex_unlock(mutex + tim_dat->id);
					printf("Machine started successfully\n");
				}
			else if (!strcmp(file_name, END_FILE_NAME))
				{
					system(poweroff);
					printf("Data processed successfully\n");
					return(1);
				}
		}
	return (0);
}

int init_inotify(char *notification_dir)
{
  int		fd_notify;

  if ((fd_notify = inotify_init()) == -1)
  	{
  	  printf("Can not init inotify");
  	  exit(2);
  	}
	if (mkdir(notification_dir, ACCESSPERMS))
		{
			printf("Directory %s already exists\n", notification_dir);
		}
  if (inotify_add_watch(fd_notify, notification_dir, IN_CLOSE_WRITE) == -1)
  	{
  	  printf("Can not add watcher");
  	  exit(2);
  	}
	return (fd_notify);
}
