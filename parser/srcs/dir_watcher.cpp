// ************************************************************************** //
//                                                                            //
//                                                                            //
//   dir_watcher.cpp                                                          //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/09 17:10:12 by Mateo                                    //
//   Updated: 2019/02/05 11:35:01 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <sys/inotify.h>
#include <fcntl.h>

// constructor empty
dir_watcher::dir_watcher()
{
}

// consruct to watch the specified path
dir_watcher::dir_watcher(const std::string path)
{
	this->path = path;
	// init inotify
  if ((fd_notify = inotify_init()) == -1)
  	{
  	  printf("Can not init inotify");
  	  exit(2);
  	}
	// set connection non blocking to continue if no chnage has been made
	if (fcntl(fd_notify, F_SETFL, O_NONBLOCK) < 0)
		{
			dprintf(2,"Error: setting fd flag\n");
			exit(EXIT_FAILURE);
		}
	// set up the watcher
  if (inotify_add_watch(fd_notify, path.c_str(), IN_CLOSE_WRITE) == -1)
  	{
  	  dprintf(2, "Can not add watcher\nMay be %s does not exists?\n", path.c_str());
  	  exit(2);
  	}
}

// read the events from file descriptor
char *dir_watcher::watch_directory(char *name)
{
	static unsigned int pos = 0;
	static char buff[INOTIFY_BUFF];
	int readed;
	struct inotify_event *event;

	// if there is an event inside the previous buff readed
	if (pos >= sizeof(struct inotify_event))
		{
			// get the event structure
			event = (struct inotify_event *)buff;
			// The name of the event is after the structure (C struct hack)
			// so if the name is complete read it
			if (pos >= sizeof(struct inotify_event) + event->len)
				{
					strcpy(name, event->name);
					pos -= sizeof(struct inotify_event) + event->len;
					memcpy(buff, buff + sizeof(struct inotify_event) + event->len, pos);
					return (name);
				}
		}
	// read the next events and return the name, (INOTIFY_BUFF will have always the size for at least one structure and his name)
	if ((readed = read(fd_notify, buff + pos, INOTIFY_BUFF - pos)) > 0)
		{
			event = (struct inotify_event *)buff;
			strcpy(name, event->name);
			pos = readed - (sizeof(inotify_event) + event->len);
			// move the buff so the next read can complete the remminent if there is
			memcpy(buff, buff + sizeof(inotify_event) + event->len, pos);
			return(name);
		}
	else
		return (NULL);
}
