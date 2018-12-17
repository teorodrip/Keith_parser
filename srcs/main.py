from srcs.header import *
 
def thread_main(path=FILE_PATH, pid=0, index=0, n_threads=0):
    app = xw.apps[pid]
    ex = excel(app, path, 0)
    ex.del_book(0)
    n_tickers = len(tickers)
    print("Got %d tickers." % n_tickers)
    tick_div =  n_tickers // n_threads
    row_remaining = (tick_div % SHEET_ROWS)
    tick_div = tick_div - row_remaining
    start = index * tick_div
    end  = tick_div * index + tick_div
    while start < end:
        print("Writing(" + str(index * tick_div) + "-" + str(end) + "): [" + str(start) + ":" + str(start + SHEET_ROWS) + "] -> ")# + str(tickers[start:inc]))
        ex.write_col("A4", tickers[start:(start + SHEET_ROWS)])
#         time.sleep(3)
        start += SHEET_ROWS
    if index == (n_threads - 1):
        remaining = end + (n_tickers % n_threads) + (row_remaining * n_threads)
        print("Remaining: %d" % remaining)
        tmp = end
        while end < remaining:
            tmp += SHEET_ROWS
            if tmp > remaining:
                tmp = remaining
            print("Writing("  + str(tick_div * index + tick_div) + "-" + str(remaining) + "): [" + str(end) + ":" + str(tmp) + "] -> ")# + str(tickers[start:inc]))
            ex.write_col("A4", tickers[end:tmp])
#             time.sleep(3)
            end = tmp

    #TODO mirar el rango con la division y el resto
    #tickers_range = range(tick_div * index, tick_div * index + tick_div + remainder, SHEET_ROWS)
    #time.sleep(100)
#     if len(tickers) % SHEET_ROWS:
#         print("Writing: [" + str(start) + ":" + str(len(tickers)) + "] -> ")# + str(tickers[start:end]))
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
    
def init_threads(n_threads=0, daem=False):
    for i in range(n_threads):
        try:
            app = xw.App()
            #app.screen_updating = False
            app.visible = False
            pid = app.pid
            thr = threading.Thread(target=thread_main, args=[FILE_PATH, pid, i, n_threads])
            app_arr.append(app)
            pid_arr.append(pid)
            threads.append(thr)
            thr.daemon = daem
            thr.start()
            print("Created thread %d associated with Excel pid %d" % (i, pid))
        except ValueError:
            print(ValueError)

if __name__ == '__main__':
    tickers = get_tickers()
    init_threads(N_THREADS, daem=False)