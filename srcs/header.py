import xlwings as xw
import threading
from excel import excel
from class_sql_db import ClassSqlDb
import time
import ast

# MACROS
##########################################################################################

N_THREADS = 1
FILE_PATH = r"C:\Users\unchartech\Desktop\Croissance_Marges_100.xlsb"
NOTIFICATION_FILE_PATH = r"Z:\keith_parser\notifications\reboot.log"
THREAD_FILE_PATH = r"Z:\keith_parser\notifications"
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
CIQINACTIVE_RTETRIES = 0
INVALID_IDNENTIFIER_RETRIES = 2
REFRESH_RETRIES = 3
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
tickers = []
hash_retries = {}
lock = threading.Lock()
reboot_flag = [False]
ObjDB = ClassSqlDb(DB_NAME, DB_USER, DB_PASSWORD, DB_HOST)

# FUNCTIONS
##########################################################################################

def print_thread_info(index, tmp, tick_div, retrial_tickers, tickers_to_delete, end):
		print("===========================\n Thread %d\n===========================\nOK => [%d/%d]\nRetry => [%d/%d]\nDelete => [%d/%d]\n===========================" % (index, tmp - (index * tick_div) - len(retrial_tickers) - len(tickers_to_delete), end - (index * tick_div), len(retrial_tickers), end - (index * tick_div), len(tickers_to_delete), end - (index * tick_div)))


def get_tickers(remainings=False):
	if remainings:
		request = SQL_REM_REQ
	else:
		request = SQL_ALL_REQ
	try:
		results = ObjDB.method_select(str_advanced_request=request)
		results = [row[0] for row in results]
		return (results)
	except ValueError:
		print(ValueError)
		

def read_thread_file(file_name):
	try:
		thread_file = open(file_name, "r")
		start = int(thread_file.readline())
		retrial_tickers = ast.literal_eval(thread_file.read())
		print("Start: " + str(start))
		print("Tickers:" + str(retrial_tickers))
		thread_file.close()
		return (start, retrial_tickers)
	except FileNotFoundError:
		return (None, None)
	except ValueError:
		return (None, None)

def write_reboot_file(f):
	try:
		file = open(NOTIFICATION_FILE_PATH, "w")
		file.write(f)
		file.close()
	except ValueError:
		print(ValueError)

def write_thread_file(file_name, start, retrial_tickers):
	try:
		thread_file = open(file_name, "w")
		thread_file.write(str(start) + "\n" + str(retrial_tickers))
		thread_file.close()
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

def parse_data (data, retrial_tickers, tickers_to_delete, final_data):
	global reboot_flag
	data_rows = len(data)
	error_len = len(ERROR_ARR)
	retrial_tickers_copy = retrial_tickers.copy()

	for i in range(data_rows):
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
		if error and (data[i][0] not in retrial_tickers_copy):
			retrial_tickers_copy.append(data[i][0])
		elif (not error):
			if data[i][0] in retrial_tickers_copy:
				retrial_tickers_copy.remove(data[i][0])
			final_data.append(data[i])
	retrial_tickers[:] = retrial_tickers_copy



		# if ERR_CIQ in data[i] or True:
		# 	if hash_retries[data[i][0]][0] < CIQINACTIVE_RTETRIES and False:
		# 		hash_retries[data[i][0]][0] += 1
		# 		error = True
		# 		print("Error in " + data[i][0] + " [ERR_CIQ] => " + str(hash_retries[data[i][0]]))
		# 	else:
		# 		print("Rebooting VM")
		# 		lock.aquire()
		# 		reboot_flag = True
		# 		lock.release()
		# if ERR_INV in data[i]:
		# 	if hash_retries[data[i][0]][1] < INVALID_IDNENTIFIER_RETRIES:
		# 		hash_retries[data[i][0]][1] += 1
		# 		error = True
		# 		print("Error in " + data[i][0] + " [ERR_INV] => " + str(hash_retries[data[i][0]]))
		# 	else:
		# 		tickers_to_delete.append(data[i][0])
		# if ERR_REF in data[i]:
		# 	if hash_retries[data[i][0]][2] < REFRESH_RETRIES:
		# 		hash_retries[data[i][0]][2] += 1
		# 		error = True
		# 		print("Error in " + data[i][0] + " [ERR_REF] => " + str(hash_retries[data[i][0]]))
		# 	else:
		# 		reboot_vm()
		# if error and (data[i][0] not in retrial_tickers):
		# 	retrial_tickers.append(data[i][0])
		# elif (not error):
		# 	if data[i][0] in retrial_tickers:
		# 		retrial_tickers.remove(data[i][0])
		# 	final_data.append(data[i])
