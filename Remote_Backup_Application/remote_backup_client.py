from tkinter import *
from tkinter.ttk import *
import socket
import struct
import os
import pickle
import time
import threading
import tarfile,tempfile

def get_files_info(path):
    if not path or not os.path.exists(path):
        return None
    files = os.walk(path)
    infos = []
    file_paths = []
    for p,ds,fs in files:
        for f in fs:
            file_name = os.path.join(p,f)
            file_size = os.stat(file_name).st_size
            file_paths.append(file_name)
            file_name = file_name[len(path)+1:]
            infos.append((file_size,file_name))
    return infos,file_paths

def send_files_infos(my_sock,infos,compress):
    fmt_str = 'Q?'
    infos_bytes = pickle.dumps(infos)
    infos_bytes_len = len(infos_bytes)
    infos_len_pack = struct.pack(fmt_str,infos_bytes_len,compress)
    my_sock.sendall(infos_len_pack)
    my_sock.sendall(infos_bytes)

def send_files(my_sock,file_path,compress):
    if not compress:
        f = open(file_path,'rb')
    else:
        f = tempfile.NamedTemporaryFile()
        tar = tarfile.open(mode='w|gz',fileobj=f)
        tar.add(file_path)
        tar.close()
        f.seek(0)
        filesize = os.stat(f.name).st_size
        filesize_bytes = struct.pack('Q',filesize)
        my_sock.sendall(filesize_bytes)
    try:
        while True:
            data = f.read(1024)
            if data:
                my_sock.sendall(data)
            else:
                break
    finally:
        f.close()


def get_bak_info(my_sock,size=7):
    info = my_sock.recv(size)
    print(info.decode('utf-8'))

def start(host,port,src,compress):
    if not os.path.exists(src):
        print("Target file not exists.")
        return
    s = socket.socket()
    s.connect((host,port))
    path = src
    file_infos,file_paths = get_files_info(path)
    send_files_infos(s,file_infos,compress)
    for fp in file_paths:
        send_files(s,fp,compress)
        print(fp)
        get_bak_info(s)
    s.close()

class MyFrame(Frame):

    def __init__(self,root):
        super().__init__(root)
        self.root = root
        self.grid()
        self.remote_ip = '127.0.0.1'
        self.remote_ports = 10888
        self.remote_ip_var = StringVar()
        self.remote_ports_var = IntVar()
        self.bak_src_var = StringVar()
        self.compress_var = BooleanVar()
        self.init_components()

    def init_components(self):
        proj_name = Label(self,text="Remote Backup - Client")
        proj_name.grid(columnspan=2)

        serv_ip_label = Label(self,text="Service IP：")
        serv_ip_label.grid(row=1, sticky=E)

        self.serv_ip = Entry(self,textvariable=self.remote_ip_var)
        self.remote_ip_var.set(self.remote_ip)
        self.serv_ip.grid(row=1,column=1)

        serv_port_label = Label(self,text="Service Port：")
        serv_port_label.grid(row=2, sticky=E)

        self.serv_port = Entry(self,textvariable=self.remote_ports_var)
        self.remote_ports_var.set(self.remote_ports)
        self.serv_port.grid(row=2,column=1)

        src_label = Label(self,text="Files：")
        src_label.grid(row=3, sticky=E)

        self.bak_src = Entry(self,textvariable=self.bak_src_var)
        self.bak_src.grid(row=3,column=1)

        tar_label = Label(self,text="Compress：")
        tar_label.grid(row=4, sticky=E)

        self.compress_on = Checkbutton(self,text="Yes",variable=self.compress_var,
            onvalue=1,offvalue=0)
        self.compress_on.grid(row=4,column=1)

        self.start_serv_btn = Button(self,text="Back-Up",command=self.start_send)
        self.start_serv_btn.grid(row=5)

        self.start_exit_btn = Button(self,text="Exit",command=self.root.destroy)
        self.start_exit_btn.grid(row=5,column=1)

    def start_send(self):
        # print(self.remote_ip_var.get(),self.remote_ports_var.get())
        # print('start...')
        host = self.remote_ip_var.get()
        port = self.remote_ports_var.get()
        compress = self.compress_var.get()
        src = self.bak_src_var.get()
        self.bak_src_var.set('')
        t = threading.Thread(target=start,args=(host,int(port),src,compress))
        t.start()
        # start(self.remote_ip_var.get(),int(self.remote_ports_var.get()),self.bak_src_var.get(),compress)

if __name__ == '__main__':
    root = Tk()
    root.title('Client')
    root.resizable(False,False)
    app = MyFrame(root)
    app.mainloop()