from header import *
 
def thread_main(path=FILE_PATH, pid=0, index=0, n_threads=0):
	app = xw.apps[pid]
	ex = excel(app, path, 0)
	ex.del_book(0)
	n_tickers = len(tickers)
	tick_div =  n_tickers // n_threads
	start = index * tick_div
	end  = tick_div * index + tick_div

	if index == (n_threads - 1):
		end += (n_tickers % n_threads)

	tmp = start
	retrial_tickers = []
	tickers_to_delete = []
	final_data = []
	while (start < end) or (len(retrial_tickers) != 0):
		tmp += (DATA_ROWS - len(retrial_tickers))
		if tmp > end:
			tmp = end
		ticker_col = retrial_tickers + tickers[start:tmp]
		ex.write_col(DATA_BEGIN, ticker_col)
		data = ex.get_range(ex.sheet.range((4, 1), (SHEET_ROWS, SHEET_COLS)))
		parse_data(data, retrial_tickers, tickers_to_delete, final_data)
		start = tmp
		print("===========================\n Thread %d\n===========================\nOK => [%d/%d]\nRetry => [%d/%d]\nDelete => [%d/%d]\n===========================" % (index, tmp - (index * tick_div) - len(retrial_tickers) - len(tickers_to_delete), end - (index * tick_div), len(retrial_tickers), end - (index * tick_div), len(tickers_to_delete), end - (index * tick_div)))

def init_threads(n_threads=0, daem=False):
	for i in range(n_threads):
		try:
			app = xw.App()
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
	for i in range(n_threads):
		try:
			threads[i].join()
		except ValueError:
			print(ValueError)
	print("Total tickers processed well: %d\nTotal tickers to delete: %d" % (tickers_ok, tickers_delete))

if __name__ == '__main__':
	start_time = time.time()
	tickers = get_tickers()
	print("Got %d tickers." % len(tickers))
	for ticker in tickers:
		hash_retries.update({ticker: [0, 0, 0]})
	init_threads(N_THREADS, daem=False)
	print("Total time: " + str(time.time() - start_time))
