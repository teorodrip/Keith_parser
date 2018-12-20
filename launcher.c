/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   launcher.c                                                               */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2018/12/20 17:48:49 by Mateo                                    */
/*   Updated: 2018/12/20 18:59:36 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NAME_MAX 100
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define FILE_PATH "./notification.log"

 static void displayInotifyEvent(struct inotify_event *i)
 {
     printf("    wd =%2d; ", i->wd);
     if (i->cookie > 0)
         printf("cookie =%4d; ", i->cookie);

     printf("mask = ");
     if (i->mask & IN_ACCESS)        printf("IN_ACCESS ");
     if (i->mask & IN_ATTRIB)        printf("IN_ATTRIB ");
     if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
     if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
     if (i->mask & IN_CREATE)        printf("IN_CREATE ");
     if (i->mask & IN_DELETE)        printf("IN_DELETE ");
     if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
     if (i->mask & IN_IGNORED)       printf("IN_IGNORED ");
     if (i->mask & IN_ISDIR)         printf("IN_ISDIR ");
     if (i->mask & IN_MODIFY)        printf("IN_MODIFY ");
     if (i->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF ");
     if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
     if (i->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
     if (i->mask & IN_OPEN)          printf("IN_OPEN ");
     if (i->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
     if (i->mask & IN_UNMOUNT)       printf("IN_UNMOUNT ");
     printf("\n");

     if (i->len > 0)
         printf("        name = %s\n", i->name);
 }

int main()
{
  int		fd_notify;
  size_t	readed;
  char		buff[BUF_LEN];
  char		*buff_tmp;
  struct inotify_event *event;

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
		  displayInotifyEvent(event);
		  buff_tmp += sizeof(struct inotify_event) + event->len;
		}
	}
  return (0);
}
