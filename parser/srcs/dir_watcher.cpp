// ************************************************************************** //
//                                                                            //
//                                                                            //
//   dir_watcher.cpp                                                          //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/09 17:10:12 by Mateo                                    //
//   Updated: 2019/01/17 16:13:41 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <sys/inotify.h>
#include <fcntl.h>

dir_watcher::dir_watcher()
{
}

dir_watcher::dir_watcher(const std::string path)
{
	this->path = path;
  if ((fd_notify = inotify_init()) == -1)
  	{
  	  printf("Can not init inotify");
  	  exit(2);
  	}
	if (fcntl(fd_notify, F_SETFL, O_NONBLOCK) < 0)
		{
			dprintf(2,"Error: setting fd flag\n");
			exit(EXIT_FAILURE);
		}
  if (inotify_add_watch(fd_notify, path.c_str(), IN_CLOSE_WRITE) == -1)
  	{
  	  printf("Can not add watcher");
  	  exit(2);
  	}
}

char *dir_watcher::watch_directory(char *name)
{
	static unsigned int pos = 0;
	static char buff[INOTIFY_BUFF];
	int readed;
	struct inotify_event *event;

	if (pos >= sizeof(struct inotify_event))
		{
			event = (struct inotify_event *)buff;
			if (pos >= sizeof(struct inotify_event) + event->len)
				{
					strcpy(name, event->name);
					pos -= sizeof(struct inotify_event) + event->len;
					memcpy(buff, buff + sizeof(struct inotify_event) + event->len, pos);
					return (name);
				}
		}
	if ((readed = read(fd_notify, buff + pos, INOTIFY_BUFF - pos)) > 0)
		{
			event = (struct inotify_event *)buff;
			strcpy(name, event->name);
			pos = readed - (sizeof(inotify_event) + event->len);
			memcpy(buff, buff + sizeof(inotify_event) + event->len, pos);
			return(name);
		}
	else
		return (NULL);
}
