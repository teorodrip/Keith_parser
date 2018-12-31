from header import *
 
def thread_main(path=FILE_PATH, pid=0, index=0, n_threads=0):
	global reboot_flag
	global tickers
	app = xw.apps[pid]
	ex = excel(app, path, 0)
	ex.del_book(0)
	tickers_to_delete = []
	final_data = []

	while not tickers.empty():
		ticker_col = get_ticker_batch(DATA_ROWS)
		ex.write_col(DATA_BEGIN, ticker_col)
		data = ex.get_range(ex.sheet.range((4, 1), (SHEET_ROWS, SHEET_COLS)))
		parse_data(data, tickers_to_delete, final_data)
		final_data.clear()#upload data here
		if reboot_flag[0]:
			print("Finishing thread %d for reboot" % (index))
			return
		print_thread_info(index, tickers.qsize())

def init_threads(n_threads=0, daem=False):
	for i in range(n_threads):
		try:
			app = xw.App()
			app.visible = APP_VISIBLE
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

if __name__ == '__main__':
	global reboot_flag
	global tickers

	write_start_file()
	get_tickers()
	print("Got %d tickers." % tickers.qsize())
	init_threads(N_THREADS, daem=False)
	if reboot_flag[0]:
		write_tickers_file()
		write_reboot_file()
	else:
		write_end_file()
