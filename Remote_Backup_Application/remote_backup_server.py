from tkinter import *
from tkinter.ttk import *
import socket
import struct
import os
import pickle
import threading

BAK_PATH = r'd:\files_server'

SERV_RUN_FLAG = True
flag_lock = threading.Lock()

def recv_unit_data(clnt,infos_len):
    data = b''
    if 0 < infos_len <= 1024:
        data += clnt.recv(infos_len)
    else:
        while True:
            if infos_len > 1024:
                data += clnt.recv(1024)
                infos_len -= 1024
            else:
                data += clnt.recv(infos_len)
                break
    return data

def get_files_info(clnt):
    fmt_str = 'Q?'
    headsize = struct.calcsize(fmt_str)
    data = clnt.recv(headsize)
    infos_len,compress = struct.unpack(fmt_str,data)
    data = recv_unit_data(clnt,infos_len)
    return pickle.loads(data),compress

def mk_path(filepath):
    paths = filepath.split(os.path.sep)[:-1]
    p = BAK_PATH
    for path in paths:
        p = os.path.join(p,path)
        if not os.path.exists(p):
            os.mkdir(p)

def get_compress_size(clnt):
    fmt_str = "Q"
    size = struct.calcsize(fmt_str)
    data = clnt.recv(size)
    size, = struct.unpack(fmt_str,data)
    return size

def recv_file(clnt,infos_len,filepath,compress):
    mk_path(filepath)
    filepath = os.path.join(BAK_PATH,filepath)
    if compress:
        infos_len = get_compress_size(clnt)
        filepath = ''.join([os.path.splitext(filepath)[0],'.tar.gz'])
    f = open(filepath,'wb+')
    try:
        if 0 < infos_len <= 1024:
            data = clnt.recv(infos_len)
            f.write(data)
        else:
            while True:
                if infos_len > 1024:
                    data = clnt.recv(1024)
                    f.write(data)
                    infos_len -= 1024
                else:
                    data = clnt.recv(infos_len)
                    f.write(data)
                    break
    except:
        print('error')
    else:
        return True
    finally:
        f.close()

def send_echo(clnt,res):
    if res:
        clnt.sendall(b'success')
    else:
        clnt.sendall(b'failure')

def client_operate(client):
    files_lst,compress = get_files_info(client)
    for size,filepath in files_lst:
        res = recv_file(client,size,filepath,compress)
        send_echo(client,res)
    client.close()

def start(host,port):
    print('start...')
    if not os.path.exists(BAK_PATH):
        os.mkdir(BAK_PATH)
    st = socket.socket()
    st.settimeout(1)
    st.bind((host,port))
    st.listen(1)
    flag_lock.acquire()
    while SERV_RUN_FLAG:
        flag_lock.release()
        client = None
        try:
            client,addr = st.accept()
        except socket.timeout:
            pass
        if client:
            t = threading.Thread(target=client_operate,args=(client,))
            t.start()
        flag_lock.acquire()
    st.close()


class MyFrame(Frame):

    def __init__(self,root):
        super().__init__(root)
        self.root = root
        self.grid()
        self.local_ip = '127.0.0.1'
        self.serv_ports = [10888,20888,30888]
        self.init_components()

    def init_components(self):
        proj_name = Label(self,text="Remote Backup - Server")
        proj_name.grid(columnspan=2)

        serv_ip_label = Label(self,text="Service IP:")
        serv_ip_label.grid(row=1)

        self.serv_ip = Combobox(self,values=self.get_ipaddr())
        self.serv_ip.set(self.local_ip)
        self.serv_ip.grid(row=1,column=1)

        serv_port_label = Label(self,text="Service Port:")
        serv_port_label.grid(row=2)

        self.serv_port = Combobox(self,values=self.serv_ports)
        self.serv_port.set(self.serv_ports[0])
        self.serv_port.grid(row=2,column=1)

        self.start_serv_btn = Button(self,text="Start Service",command=self.start_serv)
        self.start_serv_btn.grid(row=3)

        self.start_exit_btn = Button(self,text="End Service",command=self.root.destroy)
        self.start_exit_btn.grid(row=3,column=1)

    def get_ipaddr(self):
        host_name = socket.gethostname()
        info = socket.gethostbyname_ex(host_name)
        info = info[2]
        info.append(self.local_ip)
        return info

    def start_serv(self):
        # print(self.serv_ip.get(),self.serv_port.get())
        # start(self.serv_ip.get(),int(self.serv_port.get()))
        host = self.serv_ip.get()
        port = int(self.serv_port.get())
        serv_th = threading.Thread(target=start,args=(host,port))
        serv_th.start()
        self.start_serv_btn.state(['disabled',])

class MyTk(Tk):
    def destroy(self):
        global SERV_RUN_FLAG
        while True:
            if flag_lock.acquire():
                SERV_RUN_FLAG = False
                flag_lock.release()
                break
        super().destroy()

if __name__ == '__main__':
    root = MyTk()
    root.title('Server')
    root.resizable(False,False)
    app = MyFrame(root)
    app.mainloop()