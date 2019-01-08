/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   launcher.h                                                               */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/02 13:45:42 by Mateo                                    */
/*   Updated: 2019/01/07 15:27:33 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include </usr/include/postgresql/libpq-fe.h>

#define VM_NB 1
#define VM_NAME_1 "windows_1"
#define VM_NAME_2 "windows_2"
#define VM_ARR {VM_NAME_1}//, VM_NAME_2}

#define WRITE_BUFF 1024
#define BATCH_SIZE 100
#define DB_NAME "infrastructure"
#define DB_USER "user_data_read_create"
#define DB_PASS "user_data_read_create"
#define DB_HOST "192.168.27.122"
#define DB_TIMOUT "10" //Max
#define SQL_ALL_REQ "SELECT A.tickers FROM data.test_gregoire_ticker_list A\nLEFT JOIN (\nSELECT DISTINCT ON (ticker) ticker, event_time\nFROM data.test_gregoire_growth_margin\nORDER BY ticker, event_time DESC\n) B on A.tickers = B.ticker\nORDER BY event_time ASC NULLS FIRST"

#define NAME_MAX 100
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define REBOOT_FILE_NAME "reboot.log"
#define START_FILE_NAME "start.log"
#define END_FILE_NAME "end.log"
#define TICKERS_PATH "./tickers/"
#define CLEAN_COMMAND "rm -rf notifications*"
#define TIME_OUT 90

typedef struct vm_data_s
{
	unsigned char id;
	char start[NAME_MAX];
	char reboot[NAME_MAX];
	char poweroff[NAME_MAX];
	char notification_dir[NAME_MAX];
} vm_data_t;

typedef struct tickers_s
{
	uint32_t len;
	char **tickers;
} tickers_t;

pthread_mutex_t mutex[VM_NB];
unsigned char start_success[VM_NB];

void *launcher(void *arg);
void get_data(PGconn *conn, char *request, tickers_t *tickers);
void clean_tickers(tickers_t *tickers);
PGconn *connect_db(const char *db_name,
									 const char *db_user,
									 const char *db_pass,
									 const char *db_host);
#endif
