from header import *
 
def calculate_sheet():
	global reboot_flag
	global tickers

	app = xw.App()
	app.visible = APP_VISIBLE
	ex = excel(app, FILE_PATH, 0)
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
			print("Finishing for reboot")
			return
		print_thread_info(tickers.qsize())

if __name__ == '__main__':
	global reboot_flag
	global tickers

	write_start_file()
	get_tickers()
	print("Got %d tickers." % tickers.qsize())
	calculate_sheet()
	if reboot_flag[0]:
		write_tickers_file()
		write_reboot_file()
	else:
		write_end_file()
