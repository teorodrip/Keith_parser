import xlwings as xw
import sys

class excel:
    app = None
    book = None
    sheet = None
    
    def __init__(self, app=None, book=None, sheet=None):
        self.app = app
        self.book = book
        self.sheet = sheet

    def init_book(self, path):
        if self.app is not None:
            try:
                self.book = self.app.books.open(path)
            except ValueError:
                print(ValueError)
                self.del_all_books()
                sys.exit()
        else:
            print("Error: Can not load book because app is not initialized\n")
            self.del_all_books()
            sys.exit()
    
    def del_book(self, nb):
        if (self.app != None):
            try:
                self.app.books[nb].close()
            except ValueError:
                print(ValueError)
        else:
            print("Error: Can not close book because app is not initialized\n")
            self.del_all_books()
            sys.exit()
            
    def del_all_books(self):
        if (self.app != None):
            try:
                for i in self.app.books:
                    i.close()
            except ValueError:
                print(ValueError)
                sys.exit()
        else:
            print("Error: Can not close book because app is not initialized\n")
            sys.exit()
            
    def init_sheet(self, nb=0):
        if self.book is not None:
            try:
                self.sheet = self.book.sheets[nb]
            except ValueError:
                print(ValueError)
                self.del_all_books()
                sys.exit()
        else:
            print("Error: Can not load sheet because book is not initialized\n")
            self.del_all_books()
            sys.exit()
        