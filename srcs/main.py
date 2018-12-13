import xlwings as xw
import threading
from excel import excel

FILE_PATH = r"C:\Users\stagiaire3\Desktop\workspace\data_scrapping\Excel_scrapper\Keith_parser\Croissance_Marges_100.xlsb"

app_arr = []
pid_arr = []
threads = []
 
def thread_main(path=FILE_PATH, pid=0):
    app = xw.apps[pid]
    ex = excel(app=app)
    #ex.init_book(path)
    ex.del_book(0)
    ex.init_sheet(0)
    ex.sheet.range('A1').value = 47823
    
def init_threads(n_threads=8, daem=False):
    for i in range(n_threads):
        try:
            app = xw.App()
            pid = app.pid
            thr = threading.Thread(target=thread_main, args=[FILE_PATH, pid])
            app_arr.append(app)
            pid_arr.append(pid)
            threads.append(thr)
            thr.daemon = daem
            thr.start()
            print("Created thread %d associated with Excel pid %d\n" % (i, pid))
        except ValueError:
            print(ValueError)

if __name__ == '__main__':
    init_threads(2, daem=False)