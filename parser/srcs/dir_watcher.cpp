// ************************************************************************** //
//                                                                            //
//                                                                            //
//   dir_watcher.cpp                                                          //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/09 17:10:12 by Mateo                                    //
//   Updated: 2019/01/11 14:19:36 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <sys/inotify.h>
#include <fcntl.h>

dir_watcher::dir_watcher()
{
}

dir_watcher::dir_watcher(const unsigned char id)
{
	path = DEFAULT_PATH + std::to_string(id) + "/";
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

void dir_watcher::watch_directory()
{
	int readed;
	char buff[BUFF_SIZE * sizeof(struct inotify_event)];
	char *buff_tmp;
	struct inotify_event *event;

	while ((readed = read(fd_notify, buff, BUFF_SIZE)) > 0)
		{
			for(buff_tmp = buff; buff_tmp < buff + readed;)
				{
					event = (struct inotify_event *)buff_tmp;
					manage_event(event);
					buff_tmp += sizeof(struct inotify_event) + event->len;
				}
		}

}

void dir_watcher::manage_event(struct inotify_event *event)
{
	if (event->len > 0)
		{
			printf("Parsing %s\n", event->name);
			//parse the sheet
			excel_parser ex = excel_parser(this->path + event->name);
			ex.init();
			ex.parse_book();
		}
}
