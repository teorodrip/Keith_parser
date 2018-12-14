from srcs.header import *
import time
 
def thread_main(path=FILE_PATH, pid=0, index=0):
    app = xw.apps[pid]
    ex = excel(app, path, 0)
    ex.del_book(0)
    tick_div = len(tickers) // len(threads)
    if index == (len(threads) - 1):
        remainder = len(tickers) % len(threads)
    else:
        remainder = 0
    #TODO mirar el rango con la division y el resto
    tickers_range = range(tick_div * index, tick_div * index + tick_div + remainder, SHEET_ROWS)
    start = 0
    print(tickers_range)
    for end in tickers_range:
        if end:
            print("Writing(" + str(index) + "): [" + str(start) + ":" + str(end) + "] -> ")# + str(tickers[start:end]))
        start = end
    if len(tickers) % SHEET_ROWS:
        print("Writing: [" + str(start) + ":" + str(len(tickers)) + "] -> ")# + str(tickers[start:end]))
        #ex.write_col("A4", ticker_batch)
        #time.sleep(10)
#     ex.sheet.range('A1').value = 47823
#     ran = ex.sheet.range(RAN_100)
#     for i in range(200):
#         print(ran[i].raw_value)
    #ObjDB = ClassSqlDb(DB_NAME, DB_USER, DB_PASSWORD, DB_HOST)
    #results = ObjDB.method_select(str_advanced_request=SQL_ALL_REQ)
    #results = [row[0] for row in results]
    #ex.write_col("A4", tickers)
    
def init_threads(n_threads=8, daem=False):
    for i in range(n_threads):
        try:
            app = xw.App()
            pid = app.pid
            thr = threading.Thread(target=thread_main, args=[FILE_PATH, pid, i])
            app_arr.append(app)
            pid_arr.append(pid)
            threads.append(thr)
            thr.daemon = daem
            thr.start()
            print("Created thread %d associated with Excel pid %d\n" % (i, pid))
        except ValueError:
            print(ValueError)

if __name__ == '__main__':
    n_threads = 2
    tickers = get_tickers()
    init_threads(n_threads, daem=False)