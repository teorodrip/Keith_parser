/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   launcher.c                                                               */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2018/12/20 17:48:49 by Mateo                                    */
/*   Updated: 2018/12/21 10:32:24 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NAME_MAX 100
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define FILE_PATH "./notification.log"

int main()
{
  int		fd_notify;
  size_t	readed;
  char	buff[BUF_LEN];
  char	*buff_tmp;
  struct inotify_event	*event;

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
		  buff_tmp += sizeof(struct inotify_event) + event->len;
		}
	}
  return (0);
}
