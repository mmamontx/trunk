import datetime
import tkinter as tk
from tkinter import filedialog
from PIL import ImageTk, Image

from tkinter import ttk

import mysql.connector
from mysql.connector import errorcode

DB_NAME = "stock"
INSTOCK = "instock"
OPS = "ops"
META = "meta"

query_ops = ("INSERT INTO ops (datetime, id, type, name, count, price) VALUES (%s, %s, %s, %s, %s, %s)")

# TODOs:
# - Add installer;
# - Make OK buttons disabled if there is no data in entries;
# - Make editable all entries in basket;
# - Cost for add operations;
# - Focus all windows on open;
# - Fix widths for 'add'.

class basket():
    def __init__(self):
        self.items = []

    def add(self, name, price, count, cost):
        for (n, p, cnt, cst) in self.items: # TODO: Bring dict() back
            if n == name:
                self.remove(name)
                break

        self.items.append((name, price, count, cost))

    def contains(self, name):
        for (n, p, cnt, cst) in self.items:
            if n == name:
                return True
        return False

    def remove(self, name):
        i = 0
        for (n, p, cnt, cst) in self.items:
            if n == name:
                break
            else:
                i += 1

        if i < len(self.items):
            self.items.pop(i)
        else:
            pass # TODO: Error

    def checkout(self):
        if len(self.items) == 0:
            return

        lastid = inc_lastid()

        total = 0
        for (name, price, count, cost) in self.items:
            query = ("UPDATE %s SET count=count-%d WHERE name='%s'" % (INSTOCK, count, name))
            cursor.execute(query)

            data_ops = (datetime.datetime.now(), lastid, 's', name, count, price)
            cursor.execute(query_ops, data_ops)

            tree_remove_count(name, count)

            total += cost

        cnx.commit()

        self.clear()

        s = datetime.datetime.now()
        s = str(s)
        text.insert('1.0', "%s #%d Sell op. Total: %d RUB\n" % (s[:-7], lastid, total))

    def clear(self):
        self.items = []

def inc_lastid(): # Increments 'lastid' and returns it
    query = ("UPDATE %s SET value=value+1 WHERE name='lastid'" % META)
    cursor.execute(query)

    query = ("SELECT * FROM %s WHERE name='lastid'" % META)
    cursor.execute(query)

    for (name, value) in cursor:
        lastid = value
        break # There should be a single entry

    cnx.commit()

    return int(lastid)

def file_export():
    f = filedialog.asksaveasfile(mode = 'w', defaultextension='.csv')
    if f is None:
        return

    query = ("SELECT * FROM %s" % OPS)
    cursor.execute(query)

    f.write("sep=,\ndatetime,id,type,name,count,price\n")
    for (datetime, id, type, name, count, price) in cursor:
        f.write("%s,%s,%s,%s,%s,%s\n" % (datetime, id, type, name, count, price))

    f.close()

def file_exit():
    cnx.close()
    root.quit()

def help_about():
    aboutwin = tk.Toplevel(root)
    aboutwin.resizable(0, 0)
    aboutwin.grab_set()
    aboutwin.focus_force()

    toplabel = tk.Label(aboutwin, text = "Dedicated to Misha & Natasha:")

    img = Image.open("about.png")
    img = ImageTk.PhotoImage(img)
    panel = tk.Label(aboutwin, image = img)
    panel.image = img

    label = tk.Label(aboutwin, text = "Version 1.0\nMikhail Mamontov 2017 mmamontx@gmail.com")

    toplabel.grid(row = 0)
    panel.grid(row = 1)
    label.grid(row = 2)

def radio_change(existing, entry_name, entry_price, box):
    e = existing.get()
    if e == 0:
        entry_name['state'] = 'normal'
        entry_price['state'] = 'normal'
        box['state'] = 'disabled'
    elif e == 1:
        entry_name['state'] = 'disabled'
        entry_price['state'] = 'disabled'
        box['state'] = 'readonly'
    else:
        pass # TODO: Error

def op_price(name, price):
    lastid = inc_lastid()

    data_ops = (datetime.datetime.now(), lastid, 'p', name, 0, price)
    cursor.execute(query_ops, data_ops)

    s = datetime.datetime.now()
    s = str(s)
    text.insert('1.0', "%s #%d Price op. %s %d\n" % (s[:-7], lastid, name, price))

    cnx.commit()

def op_rm(name, count):
    lastid = inc_lastid()

    data_ops = (datetime.datetime.now(), lastid, 'r', name, count, 0)
    cursor.execute(query_ops, data_ops)

    s = datetime.datetime.now()
    s = str(s)
    text.insert('1.0', "%s #%d Remove op. %d item(s)\n" % (s[:-7], lastid, count))

    cnx.commit()

def op_add(name, count, price):
    lastid = inc_lastid()

    data_ops = (datetime.datetime.now(), lastid, 'a', name, count, price)
    cursor.execute(query_ops, data_ops)

    s = datetime.datetime.now()
    s = str(s)
    text.insert('1.0', "%s #%d Add op. %d items(s)\n" % (s[:-7], lastid, count))

    cnx.commit()

def edit_add_ok(stockaddwin, existing, name, count, price):
    if len(name) > 0 and count.isdigit():
        count = int(count)
        if existing:
            price = get_price_by_name(name)
            price = int(price)

            query = ("UPDATE %s SET count=count+%d WHERE name='%s'" % (INSTOCK, count, name))
            cursor.execute(query)

            op_add(name, count, price) # TODO: Think how to avoid commit concealed in the child function

            tree_add_count(name, count)
        elif price.isdigit():
            price = int(price)
            query = ("INSERT INTO %s VALUES ('%s', %d, %d)" % (INSTOCK, name, count, price))
            cursor.execute(query)

            op_add(name, count, price)

            tree_add(name, count, price)

            if tree_size() == 1:
                global editmenu
                editmenu.entryconfig("Remove...", state = 'normal')
        else:
            return

        stockaddwin.destroy()
    else:
        pass # Fields are not filled. Ignoring

def edit_add():
    stockaddwin = tk.Toplevel(root)
    stockaddwin.resizable(0, 0)
    stockaddwin.grab_set()
    stockaddwin.focus_force()

    existing = tk.IntVar()
    existing.set(0)

    query = ("SELECT * FROM %s" % INSTOCK)
    cursor.execute(query)

    names = []
    for (name, count, price) in cursor:
        names.append(name)

    entry_name = tk.Entry(stockaddwin, state = 'normal', width = 23) # 23 is default width of combobox
    entry_count = tk.Entry(stockaddwin, state = 'normal', width = 23)
    entry_price = tk.Entry(stockaddwin, state = 'normal', width = 23)

    opt = tk.StringVar()
    box = ttk.Combobox(stockaddwin, textvariable = opt, state = 'disabled')
    box['values'] = names # TODO: Check the case when there is no items

    radio_new = tk.Radiobutton(stockaddwin, text = "New", variable = existing, value = 0, command = lambda : radio_change(existing, entry_name, entry_price, box))
    radio_existing = tk.Radiobutton(stockaddwin, text = "Existing", variable = existing, value = 1, command = lambda : radio_change(existing, entry_name, entry_price, box), state = 'normal' if len(names) > 0 else 'disabled')

    ok_btn = tk.Button(stockaddwin, text = "OK", width = 13, command = lambda : edit_add_ok(stockaddwin, existing.get(), opt.get() if existing.get() else entry_name.get(), entry_count.get(), entry_price.get()))

    label_count = tk.Label(stockaddwin, text = "\nCount", padx = 21) # 21 to align with names of radiobuttons
    label_price = tk.Label(stockaddwin, text = "Price", padx = 21)

    radio_new.grid(column = 0, row = 0, sticky = tk.W)
    radio_existing.grid(column = 0, row = 1, sticky = tk.W)
    entry_name.grid(column = 1, row = 0, sticky = tk.W)
    box.grid(column = 1, row = 1, sticky = tk.W)
    label_count.grid(column = 0, row = 2, sticky = tk.W)
    entry_count.grid(column = 1, row = 2, sticky = tk.S)
    label_price.grid(column = 0, row = 3, sticky = tk.W)
    entry_price.grid(column = 1, row = 3)
    ok_btn.grid(column = 1, row = 4, sticky = tk.E)

def edit_remove_ok(editremovewin, name, count, prev_count):
    if len(name) > 0 and len(count) > 0 and prev_count > 0:
        count = int(count)
        prev_count = int(prev_count)
        if count == prev_count:
            query = ("DELETE FROM %s WHERE name='%s'" % (INSTOCK, name))
            cursor.execute(query)

            op_rm(name, count)

            tree_remove(name)

            if tree_size() == 0:
                global editmenu
                editmenu.entryconfig("Remove...", state = 'disabled')
        elif count < prev_count:
            query = ("UPDATE %s SET count=count-%d WHERE name='%s'" % (INSTOCK, count, name))
            cursor.execute(query)

            op_rm(name, count)

            tree_update_count(name, prev_count - count)
        else: # count > prev_count
            return # TODO: Error

        editremovewin.destroy()
    else:
        pass # Field is not set. Ignoring

def entry_update(entry, s):
    entry.delete(0, tk.END)
    entry.insert(0, s)

def get_count_by_name(n): # TODO: Unify this function with get_price_by_name()
    query = ("SELECT * FROM %s" % INSTOCK)
    cursor.execute(query)

    rv = -1
    for (name, count, price) in cursor: # TODO: Fix ugly solution when we need to iterate until the end
        if name == n:
            rv = count

    return rv

def get_price_by_name(n):
    query = ("SELECT * FROM %s" % INSTOCK)
    cursor.execute(query)

    rv = -1
    for (name, count, price) in cursor: # TODO: Fix ugly solution when we need to iterate until the end
        if name == n:
            rv = price

    return rv

def edit_price_ok(editpricewin, name, price):
    if len(name) > 0 and len(price) > 0:
        price = int(price)
        query = ("UPDATE %s SET price=%d WHERE name='%s'" % (INSTOCK, price, name))
        cursor.execute(query)

        op_price(name, price)

        tree_update_price(name, price)

        editpricewin.destroy()
    else:
        pass

def edit_price():
    editpricewin = tk.Toplevel(root)
    editpricewin.resizable(0, 0)
    editpricewin.grab_set()
    editpricewin.focus_force()
    editpricewin.minsize(width = 201, height = editpricewin.winfo_height()) # Width 201 makes the title visible, height is default

    opt = tk.StringVar()
    box = ttk.Combobox(editpricewin, width = 30, state = 'readonly', textvariable = opt)
    query = ("SELECT * FROM %s" % INSTOCK)
    cursor.execute(query)
    box['values'] = [name for (name, count, price) in cursor]

    label_price = tk.Label(editpricewin, text = "Price")
    entry_price = tk.Entry(editpricewin, width = 16, state = "normal") # Width 16 is about a half of window size

    box.bind("<<ComboboxSelected>>", lambda event : entry_update(entry_price, get_price_by_name(opt.get())))

    ok_btn = tk.Button(editpricewin, text = "OK", width = 13, command = lambda : edit_price_ok(editpricewin, opt.get(), entry_price.get()))

    box.grid(row = 0, column = 0, columnspan = 2)
    label_price.grid(row = 1, column = 0, sticky = tk.E)
    entry_price.grid(row = 1, column = 1, sticky = tk.E)
    ok_btn.grid(row = 2, column = 1, sticky = tk.E)

def edit_remove():
    editremovewin = tk.Toplevel(root)
    editremovewin.resizable(0, 0)
    editremovewin.grab_set()
    editremovewin.focus_force()
    editremovewin.minsize(width = 201, height = editremovewin.winfo_height()) # Width 201 makes the title visible, height is default

    query = ("SELECT * FROM %s" % INSTOCK)
    cursor.execute(query)

    names = []
    for (name, count, price) in cursor:
        names.append(name)

    opt = tk.StringVar()
    box = ttk.Combobox(editremovewin, width = 30, state = 'readonly', textvariable = opt) # Width 30 to scale to window size
    box['values'] = names

    label_count = tk.Label(editremovewin, text = "Count")
    entry_count = tk.Entry(editremovewin, state = 'normal', width = 16)

    box.bind("<<ComboboxSelected>>", lambda event : entry_update(entry_count, get_count_by_name(opt.get())))

    ok_btn = tk.Button(editremovewin, width = 13, text = "OK", command = lambda : edit_remove_ok(editremovewin, opt.get(), entry_count.get(), get_count_by_name(opt.get())))

    box.grid(row = 0, column = 0, columnspan = 2)
    label_count.grid(row = 1, column = 0, sticky = tk.E)
    entry_count.grid(row = 1, column = 1, sticky = tk.E)
    ok_btn.grid(row = 2, column = 1, sticky = tk.E)

def handle_click(event, tree):
    region = tree.identify_region(event.x, event.y)
    if region == "separator":
        return "break"

def handle_open(event, tree):
    sel = tree.selection()
    item = tree.item(sel)
    item_open(item)

def basket_add(openwin, name, price, count, cost):
    if count.isdigit():
        price = int(price)
        count = int(count)
        cost = int(cost)
        if count > 0:
            b.add(name, price, count, cost)
        elif count == 0:
            b.remove(name)
        else:
            pass # TODO: Error
        openwin.destroy()
    else:
        pass

def checkout(openwin, name, price, count, cost):
    basket_add(openwin, name, price, count, cost)
    b.checkout()

def count_change(a, b, c, count, label_price, label_cost, maxcount):
    s = count.get()
    n = int(s) if s.isdigit() else 0

    if n > maxcount:
        n = maxcount
        count.set(maxcount)

    price = label_price['text']
    price = int(price)
    label_cost['text'] = "%d" % (price * n)

def item_open(item):
    openwin = tk.Toplevel(root)
    openwin.resizable(0, 0)
    openwin.focus_force()
    openwin.grab_set()

    label_title_name = tk.Label(openwin, text = 'Name')
    label_title_price = tk.Label(openwin, text = 'Price')
    label_title_count = tk.Label(openwin, text = 'Count')
    label_title_cost = tk.Label(openwin, text = 'Cost')

    if b.contains(item['text']):
        b.remove(item['text'])

    row = 1
    for (name, price, count, cost) in b.items:
        lbl_name = tk.Label(openwin, text = name)
        lbl_price = tk.Label(openwin, text = price)
        lbl_count = tk.Label(openwin, text = count)
        lbl_cost = tk.Label(openwin, text = cost)

        lbl_name.grid(row = row, column = 0)
        lbl_price.grid(row = row, column = 1)
        lbl_count.grid(row = row, column = 2)
        lbl_cost.grid(row = row, column = 3)

        row += 1

    label_name = tk.Label(openwin, text = item['text'])
    label_price = tk.Label(openwin, text = "%s" % item['values'][1])
    label_cost = tk.Label(openwin, text = "%s" % item['values'][1])

    count = tk.StringVar()
    count.set('1')
    count.trace('w', lambda a, b, c: count_change(a, b, c, count, label_price, label_cost, get_count_by_name(item['text'])))
    entry_count = tk.Entry(openwin, textvariable = count, width = 8) # Width 8 is same as buttons

    # The buttons are of the same width aligned by 'Checkout'
    back_btn = tk.Button(openwin, text = "Back", width = 8, command = lambda : basket_add(openwin, item['text'], label_price['text'], entry_count.get(), label_cost['text']))
    checkout_btn = tk.Button(openwin, text = "Checkout", width = 8, command = lambda : checkout(openwin, item['text'], label_price['text'], entry_count.get(), label_cost['text']))

    label_title_name.grid(row = 0, column = 0)
    label_title_price.grid(row = 0, column = 1)
    label_title_count.grid(row = 0, column = 2)
    label_title_cost.grid(row = 0, column = 3)

    label_name.grid(row = row, column = 0)
    label_price.grid(row = row, column = 1)
    entry_count.grid(row = row, column = 2)
    label_cost.grid(row = row, column = 3)
    back_btn.grid(row = row + 1, column = 0, sticky = tk.W)
    checkout_btn.grid(row = row + 1, column = 3)

def tree_update_price(name, price):
    global tree
    items = tree.get_children()
    for item in items:
        i = tree.item(item)
        if i['text'] == name:
            tree.item(item, text = name, values = (i['values'][0], price))
            break

def tree_update_count(name, count):
    global tree
    items = tree.get_children()
    for item in items:
        i = tree.item(item)
        if i['text'] == name:
            tree.item(item, text = name, values = (count, i['values'][1]))
            break

def tree_add_count(name, count):
    global tree
    items = tree.get_children()
    for item in items:
        i = tree.item(item)
        if i['text'] == name:
            tree.item(item, text = name, values = (i['values'][0] + count, i['values'][1]))
            break

def tree_remove_count(name, count):
    global tree
    items = tree.get_children()
    for item in items:
        i = tree.item(item)
        if i['text'] == name:
            tree.item(item, text = name, values = (i['values'][0] - count, i['values'][1]))
            break

def tree_add(name, count, price):
    global tree
    tree.insert("", tk.END, text = name, values = (count, price))

def tree_remove(name):
    global tree
    items = tree.get_children()
    for item in items:
        i = tree.item(item)
        if i['text'] == name:
            tree.delete(item)
            break

def tree_size():
    global tree
    items = tree.get_children()
    return len(items)

def tree_crt():
    global tree
    tree = ttk.Treeview(root)

    tree["columns"] = ("In stock", "Price")
    tree.heading("In stock", text = "In stock")
    tree.heading("Price", text = "Price")

    tree.bind('<Button-1>', lambda event : handle_click(event, tree))
    tree.bind('<<TreeviewOpen>>', lambda event: handle_open(event, tree))

    query = ("SELECT * FROM %s" % INSTOCK)
    cursor.execute(query)

    for (name, count, price) in cursor:
        tree.insert("", tk.END, text = name, values = (count, price))

    tree.grid()

def login_ok_btn_cmd(loginwin, entry_user, entry_pwd, stat):
    global cnx, cursor
    try:
        cnx = mysql.connector.connect(user = entry_user.get(), password = entry_pwd.get(), host = "127.0.0.1", database = DB_NAME)
    except mysql.connector.Error as err:
        if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
            stat.set("Wrong user name or password")
        elif err.errno == errorcode.ER_BAD_DB_ERROR:
            stat.set("Database does not exist")
        else:
            stat.set("Error")
        entry_pwd.delete(0, tk.END)
    else:
        cursor = cnx.cursor()
        loginwin.destroy()

        tree_crt()
        text.grid(row = 1)

        if tree_size() == 0:
            global editmenu
            editmenu.entryconfig("Remove...", state = 'disabled')

        root.focus_force()

def login():
    loginwin = tk.Toplevel(root)
    loginwin.resizable(0, 0)
    loginwin.grab_set()
    loginwin.protocol('WM_DELETE_WINDOW', root.quit)
    loginwin.wm_attributes("-topmost", 1)
    loginwin.minsize(width = 201, height = loginwin.winfo_height()) # Width 201 makes the title visible, height is default

    label_user = tk.Label(loginwin, text = "User")
    label_pwd = tk.Label(loginwin, text = "Password")

    entry_user = tk.Entry(loginwin, width = 23) # Width 23 scales the entry to the right border
    entry_pwd = tk.Entry(loginwin, show = "*", width = 23)

    stat = tk.StringVar()
    label_stat = tk.Label(loginwin, fg = "red", textvariable = stat)

    login_ok_btn = tk.Button(loginwin, text = "OK", width = 27, command = lambda : login_ok_btn_cmd(loginwin, entry_user, entry_pwd, stat)) # Width 27 stretches the button across the window

    label_user.grid(row = 0, column = 0, sticky = tk.W)
    label_pwd.grid(row = 1, column = 0)
    entry_user.grid(row = 0, column = 1)
    entry_pwd.grid(row = 1, column = 1)
    label_stat.grid(row = 2, column = 0, columnspan = 2)
    login_ok_btn.grid(row = 3, column = 0, columnspan = 2)

root = tk.Tk()
root.resizable(0, 0)
root.protocol('WM_DELETE_WINDOW', file_exit)
root.title("Punk Fiction")
menubar = tk.Menu(root)

filemenu = tk.Menu(menubar, tearoff = 0)
filemenu.add_command(label = "Export...", command = file_export)
filemenu.add_separator()
filemenu.add_command(label = "Exit", command = file_exit)
menubar.add_cascade(label = "File", menu = filemenu)

editmenu = tk.Menu(menubar, tearoff = 0)
editmenu.add_command(label = "Add...", command = edit_add)
editmenu.add_command(label = "Remove...", command = edit_remove)
editmenu.add_command(label = "Price...", command = edit_price)
menubar.add_cascade(label = "Edit", menu = editmenu)

helpmenu = tk.Menu(menubar, tearoff = 0)
helpmenu.add_command(label = "About Punk Fiction", command = help_about)
menubar.add_cascade(label = "Help", menu = helpmenu)

root.config(menu = menubar)

tree = None
text = tk.Text(root, width = 75, height = 5)

b = basket()

login()

root.mainloop()
