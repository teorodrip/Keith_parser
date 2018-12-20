import xlwings as xw
import sys

class excel:
	app = None
	book = None
	sheet = None

	def __init__(self, app=None, book_path="", sheet_nb=0):
		if app != None and book_path != "":
			self.app = app
			self.init_book(book_path)
			self.init_sheet(sheet_nb)
		elif app != None:
			self.app = app

	def init_book(self, path):
		if self.app is not None:
			try:
				self.book = self.app.books.open(path)
			except ValueError:
				print(ValueError)
				sys.exit()
		else:
			print("Error: Can not load book because app is not initialized\n")
			sys.exit()
    
	def del_book(self, nb):
		if (self.app != None):
			try:
				self.app.books[nb].close()
			except ValueError:
				print(ValueError)
		else:
			print("Error: Can not close book because app is not initialized\n")
			sys.exit()
            
	def init_sheet(self, nb=0):
		if self.book is not None:
			try:
				self.sheet = self.book.sheets[nb]
			except ValueError:
				print(ValueError)
				sys.exit()
		else:
			print("Error: Can not load sheet because book is not initialized\n")
			sys.exit()

	def write_col (self, ini_pos="A1", data=[]):
		if self.sheet != None:
			try:
				col = self.sheet.range(ini_pos)
				col.options(transpose=True).value = data
			except ValueError:
				print(ValueError)
		else:
			print("Error: Can not load column because sheet is not initialized\n")
    
	def get_range(self, data_ran):
		if self.sheet != None:
			try:
				return (data_ran.value)
			except ValueError:
				print(ValueError)
		else:
			print("Error: Can not load range because sheet is not initialized\n")
