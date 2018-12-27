from header import *
 
def thread_main(path=FILE_PATH, pid=0, index=0, n_threads=0):
	global reboot_flag
	app = xw.apps[pid]
	ex = excel(app, path, 0)
	ex.del_book(0)
	n_tickers = len(tickers)
	tick_div =  n_tickers // n_threads
	end  = tick_div * index + tick_div
	thread_file = THREAD_FILE_PATH + "\\" + str(index) + ".log"

	if index == (n_threads - 1):
		end += (n_tickers % n_threads)

	start, retrial_tickers = read_thread_file(thread_file)
	if start == None or retrial_tickers == None:
		print("First boot")
		retrial_tickers = []
		start = index * tick_div

	tmp = start
	tickers_to_delete = []
	final_data = []
	while (start < end) or (len(retrial_tickers) != 0):
		print("Reboot: " + str(reboot_flag[0]))
		tmp += (DATA_ROWS - len(retrial_tickers))
		if tmp > end:
			tmp = end
		ticker_col = retrial_tickers + tickers[start:tmp] + ([''] * (DATA_ROWS - len(retrial_tickers) - (tmp - start)))
		ex.write_col(DATA_BEGIN, ticker_col)
		data = ex.get_range(ex.sheet.range((4, 1), (SHEET_ROWS, SHEET_COLS)))
		parse_data(data, retrial_tickers, tickers_to_delete, final_data)
		final_data.clear()#upload data here
		if reboot_flag[0]:
			print("Finishing thread %d for reboot" % (index))
			time.sleep(3)
			write_thread_file(thread_file, start, retrial_tickers)
			return
		print_thread_info(index, tmp, tick_div, retrial_tickers, tickers_to_delete, end)
		start = tmp

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

if __name__ == '__main__':
	global reboot_flag

	write_start_file()
	tickers = get_tickers()
	print("Got %d tickers." % len(tickers))
	for ticker in tickers:
		hash_retries.update({ticker: [0, 0, 0]})
	init_threads(N_THREADS, daem=False)
	if reboot_flag[0]:
		write_reboot_file("r")
	else:
		write_end_file()
