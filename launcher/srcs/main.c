/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   main.c                                                                   */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/02 14:21:03 by Mateo                                    */
/*   Updated: 2019/01/03 16:19:49 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/launcher.h"

void *time_out(void *arg)
{
	time_out_data_t *tim_dat = (time_out_data_t *)arg;

  sleep(TIME_OUT);
	if (!start_success[tim_dat->id])
		{
			system(tim_dat->reboot);
			printf("Some problem booting the machine, proceding to reboot\n");
			pthread_mutex_lock(mutex + tim_dat->id);
			if (pthread_create(tim_dat->thread, NULL, time_out, tim_dat))
				{
					printf("Fatal: Can not create thread\n");
					exit(2);
				}
			pthread_mutex_unlock(mutex + tim_dat->id);
		}
	else
		{
			pthread_mutex_lock(mutex + tim_dat->id);
			start_success[tim_dat->id] = 0;
			pthread_mutex_unlock(mutex + tim_dat->id);
		}
  return (NULL);
}

int main()
{
	PGconn *conn;
	PGresult *res;
	conn = connect_db(DB_NAME, DB_USER, DB_PASS, DB_HOST);
	res = get_data(conn, SQL_ALL_REQ);
	write_tickers(res, TICKERS_PATH);
	PQclear(res);
	PQfinish(conn);
	/* pthread_t machine_watcher[VM_NB]; */
	/* char *(machine_name[]) = VM_ARR; */
	/* vm_data_t vm_data[VM_NB]; */

	/* for (int i = 0; i < VM_NB; i++) */
	/* 	{ */
	/* 		if (strlen(machine_name[i]) > NAME_MAX / 2) */
	/* 			{ */
	/* 				printf("VM name: %s is too long", machine_name[i]); */
	/* 				exit(2); */
	/* 			} */
	/* 		vm_data[i].id = i; */
	/* 		vm_data[i].start[0] = 0; */
	/* 		vm_data[i].reboot[0] = 0; */
	/* 		vm_data[i].poweroff[0] = 0; */
	/* 		vm_data[i].notification_dir[0] = 0; */
	/* 		strcat(strcat(vm_data[i].start, "VBoxManage startvm "), machine_name[i]); */
	/* 		strcat(strcat(strcat(vm_data[i].reboot, "VBoxManage controlvm "), machine_name[i]), " reset soft"); */
	/* 		strcat(strcat(strcat(vm_data[i].poweroff, "VBoxManage controlvm "), machine_name[i]), " poweroff soft"); */
	/* 		strcat(strcat(vm_data[i].notification_dir, "./notifications_"), machine_name[i]); */
	/* 		pthread_mutex_init(mutex + i, NULL); */
	/* 		start_success[i] = 0; */
	/* 		if (pthread_create(machine_watcher + i, NULL, launcher, vm_data + i)) */
	/* 			{ */
	/* 				printf("Fatal: Can not create thread\n"); */
	/* 				exit(2); */
	/* 			} */
	/* 	} */
	/* for (int i = 0; i < VM_NB; i++) */
	/* 	pthread_join(machine_watcher[i], NULL); */
	/* system(CLEAN_COMMAND); */
	/* return (0); */
}
