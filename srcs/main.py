from header import *
 
def thread_main(path=FILE_PATH, pid=0, index=0, n_threads=0):
	app = xw.apps[pid]
	ex = excel(app, path, 0)
	ex.del_book(0)
	n_tickers = len(tickers)
	print("Got %d tickers." % n_tickers)
	tick_div =  n_tickers // n_threads
	row_remaining = (tick_div % DATA_ROWS)
	tick_div = tick_div - row_remaining
	start = index * tick_div
	end  = tick_div * index + tick_div

	# Write tickers mudule DATA_ROWS
	while start < end:
		print("Writing(" + str(index * tick_div) + "-" + str(end) + "): [" + str(start) + ":" + str(start + DATA_ROWS) + "] -> ")# + str(tickers[start:inc]))
		ex.write_col(DATA_BEGIN, tickers[start:(start + DATA_ROWS)])
		data = ex.get_range(ex.sheet.range((4, 2), (SHEET_ROWS, SHEET_COLS)))
		parse_data(data, DATA_ROWS)
		start += DATA_ROWS

	# Write remaining tickers
	remaining = end + row_remaining
	if index == (n_threads - 1):
		remaining += (n_tickers % n_threads)
	print("Remaining: %d" % remaining)
	tmp = end
	while end < remaining:
		tmp += DATA_ROWS
		if tmp > remaining:
			tmp = remaining
		print("Writing("  + str(tick_div * index + tick_div) + "-" + str(remaining) + "): [" + str(end) + ":" + str(tmp) + "] -> ")# + str(tickers[start:inc]))
		ex.write_col(DATA_BEGIN, tickers[end:tmp])
		data = ex.get_range(ex.sheet.range((4, 2), (SHEET_ROWS, SHEET_COLS)))
		parse_data(data, DATA_ROWS)
		end = tmp
	app.quit()

def init_threads(n_threads=0, daem=False):
	for i in range(n_threads):
		try:
			app = xw.App()
			app.visible = True
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
	for ticker in tickers:
		hash_retries.update({ticker: [0, 0, 0]})
	init_threads(N_THREADS, daem=False)
