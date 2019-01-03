/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   launcher.c                                                               */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2018/12/20 17:48:49 by Mateo                                    */
/*   Updated: 2019/01/03 10:43:55 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/launcher.h"

void *launcher(void *arg)
{
  int		fd_notify;
  ssize_t	readed;
  char	buff[BUF_LEN];
  char	*buff_tmp;
  pthread_t time_watcher;
  struct inotify_event	*event;
	vm_data_t *vm_data = (vm_data_t *)arg;
	time_out_data_t tim_dat;
	int end = 1;

	tim_dat.thread = &time_watcher;
	tim_dat.id = vm_data->id;
	tim_dat.reboot = vm_data->reboot;
  system(vm_data->start);
  if (pthread_create(&time_watcher, NULL, time_out, (void *)&tim_dat))
		{
			printf("Fatal: Can not create thread\n");
			exit(2);
		}
	fd_notify = init_inotify(vm_data->notification_dir);
  while (end)
  	{
  	  if ((readed = read(fd_notify, buff, BUF_LEN)) < 0)
				{
					printf("Error reading");
					exit(2);
				}
  	  for (buff_tmp = buff; buff_tmp < buff + readed; )
				{
					event = (struct inotify_event *)buff_tmp;
					if (manage_event(event, &tim_dat, vm_data->poweroff))
						{
							end = 0;
							break;
						}
					buff_tmp += sizeof(struct inotify_event) + event->len;
				}
  	}
  return (0);
}
