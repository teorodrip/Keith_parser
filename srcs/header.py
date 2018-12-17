import xlwings as xw
import threading
from srcs.excel import excel
from srcs.class_sql_db import ClassSqlDb
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
RAN_100 = "B4:GB103"
SHEET_ROWS = 100

# GLOBAL
##########################################################################################
        
app_arr = []
pid_arr = []
threads = []
tickers = []
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
        break
        start += inc

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
