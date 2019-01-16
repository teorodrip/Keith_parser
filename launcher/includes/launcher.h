/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   launcher.h                                                               */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/02 13:45:42 by Mateo                                    */
/*   Updated: 2019/01/16 12:02:58 by Mateo                                    */
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
#include "./server.h"

#define VM_NB 2
#define VM_NAME_1 "windows_1"
#define VM_NAME_2 "windows_2"
#define VM_ARR {VM_NAME_1, VM_NAME_2}//, VM_NAME_2}

#define WRITE_BUFF 1024
#define BATCH_SIZE 10
#define DB_NAME "pam_test"
#define DB_USER "capiq_manager"
#define DB_PASS "capiqunchartech"
#define DB_HOST "192.168.27.122"
#define DB_TIMOUT "10" //Max
#define SQL_ALL_REQ "SELECT ticker_bbg, ticker_capiq FROM main_v2.static_inv_universe WHERE ticker_capiq IS NOT NULL AND ticker_capiq != '' ORDER BY is_invested DESC LIMIT 50"
/* #define SQL_ALL_REQ "SELECT A.tickers FROM data.test_gregoire_ticker_list A\nLEFT JOIN (\nSELECT DISTINCT ON (ticker) ticker, event_time\nFROM data.test_gregoire_growth_margin\nORDER BY ticker, event_time DESC\n) B on A.tickers = B.ticker\nORDER BY event_time ASC NULLS FIRST" */
#define PARSER_TICKERS_COL 0
#define VM_TICKERS_COL 1

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
	size_t n_tuples;
	size_t n_cols;
	size_t pos;
	unsigned char **tick_len;
	queue_t *queue;
	PGresult *res;
} tickers_t;

pthread_mutex_t mutex[VM_NB];
unsigned char start_success[VM_NB];

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN char **virtual_machines;

void *launcher(void *arg);
void get_data(PGconn *conn, char *request, tickers_t *tickers);
void clean_tickers(tickers_t *tickers);
void read_clients(client_t **head, tickers_t *tickers);
void decode_data(const char *buff, const ssize_t readed,
								 const client_t *cli, tickers_t *tickers);
PGconn *connect_db(const char *db_name,
									 const char *db_user,
									 const char *db_pass,
									 const char *db_host);
#endif
