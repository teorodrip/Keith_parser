import xlwings as xw
import threading
from excel import excel
from class_sql_db import ClassSqlDb
from xml.etree.ElementInclude import include
import time

# MACROS
##########################################################################################

N_THREADS = 1
FILE_PATH = r"C:\Users\stagiaire3\Desktop\workspace\data_scrapping\Excel_scrapper\Keith_parser\Croissance_Marges_100.xlsb"

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
DATA_COLS = 184
DATA_ROWS = 100
DATA_BEGIN = "A4"
CIQINACTIVE_RTETRIES = 3
INVALID_IDNENTIFIER_RETRIES = 2
REFRESH_RETRIES = 3
ERR_CIQ = '#CIQINACTIVE'
ERR_INV = '(Invalid Identifier)'
ERR_NA = 'NA'
ERR_NM = 'NM'
ERR_REF = '#REFRESH'


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
ObjDB = ClassSqlDb(DB_NAME, DB_USER, DB_PASSWORD, DB_HOST)

# FUNCTIONS
##########################################################################################

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
        
def write_tickers(start, end, inc, ex):
	while start < end:
		print("Writing(" + str(end) + "): [" + str(start) + ":" + str(start + inc) + "] -> ")# + str(tickers[start:inc]))
		ex.write_col("A4", tickers[start:(start + inc)])
		start += inc

def parse_data (data, row_len, start, end):
	for i in range(start, end):
		data_slice = data[((i - start) * row_len):((i - start) * row_len + row_len)]
		error = False
		if ERR_CIQ in data_slice:
			if hash_retries[i][0] < CIQINACTIVE_RTETRIES:
				hash_retries[i][0] += 1
				error = True
			# else:
			# 	reboot
		if ERR_INV in data_slice:
			if hash_retries[i][1] < INVALID_IDNENTIFIER_RETRIES:
				hash_retries[i][1] += 1
				error = True
			# else:
			# 	delete ticker from DB
		if ERR_REF in data_slice:
			if hash_retries[i][2] < REFRESH_RETRIES:
				hash_retries[i][2] += 1
				error = True
			# else:
			# 	reboot
		if error:
			tmp = tickers.pop(i)


def write_remaining_tickers(start, end, inc, ex):
	tmp = start
	while start < end:
		tmp += inc
		if tmp > end:
			tmp = end
		print("Writing(" + str(end) + "): [" + str(start) + ":" + str(tmp) + "] -> ")# + str(tickers[start:inc]))
		ex.write_col("A4", tickers[start:tmp])
		time.sleep(100)
		start = tmp
