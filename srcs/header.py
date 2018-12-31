import xlwings as xw
import threading
from excel import excel
from class_sql_db import ClassSqlDb
import time
import ast
import queue
import os

# MACROS
##########################################################################################

N_THREADS = 1
APP_VISIBLE = True
FILE_PATH = r"Z:\keith_parser\resources\Croissance_Marges_100.xlsb"
NOTIFICATION_FILE_PATH = r"Z:\keith_parser\notifications\reboot.log"
TICKERS_FILE_PATH = r"Z:\keith_parser\notifications\tickers.log"
START_FILE_PATH = r"Z:\keith_parser\notifications\start.log"
END_FILE_PATH = r"Z:\keith_parser\notifications\end.log"

# postgres parameters
DB_NAME = 'infrastructure'
DB_USER = 'user_data_read_create'
DB_PASSWORD = 'user_data_read_create'
DB_HOST = '192.168.27.122'
SQL_ALL_REQ = """
	SELECT A.tickers FROM data.test_gregoire_ticker_list A
		LEFT JOIN (
			SELECT DISTINCT ON (ticker) ticker, event_time
			FROM data.test_gregoire_growth_margin
			ORDER BY ticker, event_time DESC
		) B on A.tickers = B.ticker
		ORDER BY event_time ASC NULLS FIRST
"""
SQL_REM_REQ = """
	SELECT tickers FROM data.test_gregoire_ticker_list WHERE tickers IS NOT NULL
	EXCEPT
	SELECT ticker FROM data.test_gregoire_growth_margin
	WHERE 'now'-event_time < '10:00:00' -- 10 hours
"""
SHEET_COLS = 185
SHEET_ROWS = 103
DATA_COLS = 185
DATA_ROWS = 100
DATA_BEGIN = "A4"
CIQINACTIVE_RTETRIES = 4
INVALID_IDNENTIFIER_RETRIES = 1
REFRESH_RETRIES = 4
ERR_CIQ = '#CIQINACTIVE'
ERR_INV = '(Invalid Identifier)'
ERR_NA = 'NA'
ERR_NM = 'NM'
ERR_REF = '#REFRESH'
ERROR_ARR = [ERR_CIQ, ERR_INV, ERR_REF]
RETRY_ARR = [CIQINACTIVE_RTETRIES, INVALID_IDNENTIFIER_RETRIES, REFRESH_RETRIES]


# GLOBAL
##########################################################################################

app_arr = []
pid_arr = []
threads = []

# Each element is an array in this form:
# ["TICKER_NAME", CIQINACTIVE_retries, invalid_identifier_retries, REFRESH_retries]
# if CIQINACTIVE_retries or REFRESH_retries reaches they max retries the machine will be
# rebooted and if invalid_identifier_retries reaches his max the ticker will be removed from DB
tickers = queue.Queue()
hash_retries = {}
lock = threading.Lock()
reboot_flag = [False]
ObjDB = ClassSqlDb(DB_NAME, DB_USER, DB_PASSWORD, DB_HOST)

# FUNCTIONS
##########################################################################################

def print_thread_info(index, remaining):
		print("===========================\n Thread %d\n===========================\nRemaining: %d\n===========================" % (index, remaining))


def get_tickers(remainings=False):
	global hash_retries
	global tickers

	if os.path.exists(TICKERS_FILE_PATH) and os.path.getsize(TICKERS_FILE_PATH) > 0:
		try:
			tickers_file = open(TICKERS_FILE_PATH, "r")
			#python treats the file iterator as line by line by default
			for line in tickers_file:
				tickers.put(line)
				hash_retries.update({line: [0, 0, 0]})
		except:
			print("Error reading tickers file")
	else:
		if remainings:
			request = SQL_REM_REQ
		else:
			request = SQL_ALL_REQ
		try:
			results = ObjDB.method_select(str_advanced_request=request)
			for row in results:
				tickers.put(row[0])
				hash_retries.update({row[0]: [0, 0, 0]})
		except ValueError:
			print(ValueError)

def write_reboot_file():
	try:
		file = open(NOTIFICATION_FILE_PATH, "w")
		file.write("r")
		file.close()
	except ValueError:
		print(ValueError)

def write_tickers_file():
	global tickers

	try:
		ticker_file = open(TICKERS_FILE_PATH, "a")
		ticker_file.seek(0)
		ticker_file.truncate()
		while not tickers.empty():
			ticker_file.write("%s\n" % (tickers.get(block=False)))
		ticker_file.close()
	except ValueError:
		print(ValueError)

def write_start_file():
	try:
		file = open(START_FILE_PATH, "w")
		file.write("s")
		file.close()
	except ValueError:
		print(ValueError)

def write_end_file():
	try:
		file = open(END_FILE_PATH, "w")
		file.write("e")
		file.close()
	except ValueError:
		print(ValueError)

def get_ticker_batch(batch_size):
	global tickers
	col = []

	for _ in range(batch_size):
		try:
			col.append(tickers.get(block=False))
		except:
			return (col)
	return (col)

def parse_data (data, tickers_to_delete, final_data):
	global reboot_flag
	global tickers
	data_rows = len(data)
	error_len = len(ERROR_ARR)

	for i in range(data_rows):
		if (data[i][0] == '') or (data[i][0] is None):
			break
		data_cols = len(data[i])
		error = False
		delete = False
		for j in range(1, data_cols):
			for k in range(error_len):
				if data[i][j] == ERROR_ARR[k]:
					if hash_retries[data[i][0]][k] >= RETRY_ARR[k]:
						if ERROR_ARR[k] == ERR_INV:
							tickers_to_delete.append(data[i][0])
							delete = True
						else:
							lock.acquire()
							reboot_flag[0] = True
							lock.release()
							print("Max %s retries reached in %s, rebooting the machine" % (ERROR_ARR[k], hash_retries[data[i][0]][k]))
							tickers_to_delete.clear()
							return
					else:
						hash_retries[data[i][0]][k] += 1
						print("Error in %s {%s} => %s" % (data[i][0], ERROR_ARR[k], str(hash_retries[data[i][0]])))
						error = True
					break
			if error or delete:
				break
		if error:
			tickers.put(data[i][0])
		else:
			final_data.append(data[i])
