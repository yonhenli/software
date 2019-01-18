import os
import sqlite3
from sqlite3 import connect

class MySqliteDb(object):
    """Sqlite3 Db Class"""
    def __init__(self, dbname="address.db"):
        self.dbname = dbname
        self.con = None
        self.curs = None

    def getCursor(self):
        self.con = sqlite3.connect(self.dbname)
        if self.con:
            self.curs = self.con.cursor()

    def closeDb(self):
        if self.curs:
            self.curs.close()
        if self.con:
            self.con.commit()
            self.con.close()

    def __enter__(self):
        self.getCursor()
        return self.curs

    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_val:
            print("Exception has generate: ",exc_val)
            print("Sqlite3 execute error!")
        self.closeDb()

def initDb():
    sql_script = '''
        create table contact
        (id integer primary key autoincrement not null,
        name varchar(20) not null,
        home_tel varchar(20),
        office_tel varchar(20),
        mobile_phone varchar(20),
        memo text);
        create table mygroup(
        id integer primary key autoincrement not null,
        name  varchar(20) not null,
        memo text);
        create table con_gro(
        cid integer not null,
        gid integer not null,
        FOREIGN KEY(cid) REFERENCES contact(id),
        FOREIGN KEY(gid) REFERENCES mygroup(id)
            );
        '''
    if not os.path.exists('address.db'):
        with MySqliteDb() as db:
            db.executescript(sql_script)

class Contact:
    def __init__(self,name='',home_tel='',office_tel='',mobile_phone='',memo=''):
        self.id = -1
        self.name = name
        self.home_tel = home_tel
        self.office_tel = office_tel
        self.mobile_phone = mobile_phone
        self.memo = memo
        self.gid = None
        self.keys = ('home_tel','office_tel','mobile_phone','memo')
        self.__change()

    def save(self):
        if self.id == -1:
            param_dicts = {k:getattr(self,k) for k in self.keys if getattr(self,k)}
            keys = tuple(k for k in self.keys if k in param_dicts)
            quotes = ','.join(('?' for i in range(len(param_dicts))))
            param_tuple = [self.name,]
            for key in keys:
                param_tuple.append(param_dicts[key])
            param_tuple = tuple(param_tuple)
            sql = "insert into contact (name,%s) values (?,%s)" % (','.join(keys),quotes)
            if self.name:
                try:
                    with MySqliteDb() as db:
                        db.execute(sql,param_tuple)
                        res = db.execute('select id from contact where name=?',(self.name,))
                        res = res.fetchone()
                        self.id = res[0]
                    return True
                except:
                    print("Failed to save. Please check server.")
            else:
                print("Name cannot be empty.")
                return False
        else:
            return self.update()

    def update(self):
        sql = "update contact set %s=? where id=?"
        chgs = {k:getattr(self,k) for k in self.keys \
                if getattr(self,k) and getattr(self,k) != self.vals.get(k)}
        if not chgs:
            return
        try:
            with MySqliteDb() as db:
                for k,v in chgs.items():
                    db.execute(sql % k,(v,self.id))
            self.__change()
            return True
        except:
            print("Failed to update.")
            return False

    def load(self,id):
        try:
            with MySqliteDb() as db:
                res = db.execute('select * from contact where id=?',(id,))
                res = res.fetchone()
                self.name = res[1]
                self.id = id
                for i,v in enumerate(res[2:]):
                    setattr(self,self.keys[i-2],v)
            self.__change()
            return True
        except:
            print('Failed to load.')
        

    def load_from_name(self):
        if self.name:
            try:
                with MySqliteDb() as db:
                    res = db.execute('select * from contact where name=?',(self.name,))
                    res = res.fetchone()
                    self.id = res[0]
                    for i,v in enumerate(res[2:]):
                        setattr(self,self.keys[i-2],v)
                self.__change()
                return True
            except:
                print('Failed to load.')

    def get_by_name(self,name):
        try:
            with MySqliteDb() as db:
                res = db.execute('select * from contact where name=?',(name,))
                res = res.fetchall()
            return res
        except:
            print('Failed to query.')

    def delete(self):
        try:
            with MySqliteDb() as db:
                db.execute('delete from contact where id=?',(self.id,))
                db.execute('delete from con_gro where cid=?',(self.id,))
            return True
        except:
            print('Failed to query.')

    def all(self):
        try:
            with MySqliteDb() as db:
                res = db.execute('select * from contact')
                res = res.fetchall()
            return res
        except:
            print('Failed to query.')

    def add_to_group(self,gid):
        if gid and self.id != -1:
            self.gid = gid
            try:
                with MySqliteDb() as db:
                    db.execute('insert into con_gro (cid,gid) values (?,?)',(self.id,gid))
                return True
            except:
                print('Failed to query.')

    def __change(self):
        self.vals = {k:getattr(self,k) for k in self.keys}

class Group:
    def __init__(self,name='',memo=''):
        self.id = -1
        self.name = name
        self.memo =memo
        self.contacts = []

    def save(self):
        if self.name and self.id == -1:
            try:
                with MySqliteDb() as db:
                    db.execute('insert into mygroup (name,memo) values (?,?)',(self.name,self.memo))
                    res = db.execute('select id from mygroup where name=?',(self.name,))
                    self.id = res.fetchone()[0]
                    return True
            except:
                print("Failed to query.")
        if self.name and self.id != -1:
            return self.update()

    def update(self):
        try:
            with MySqliteDb() as db:
                db.execute('update mygroup set name=?,memo=? where id=?',(self.name,self.memo,self.id))
                return True
        except:
            print("Failed to query.")

    def load(self,id):
        try:
            with MySqliteDb() as db:
                res = db.execute('select * from mygroup where id=?',(id,))
                res = res.fetchone()
                self.id = id
                self.name = res[1]
                self.memo = res[2]
        except:
            pass

    def delete(self):
        if self.id != -1:
            try:
                with MySqliteDb() as db:
                    db.execute('delete from mygroup where id=?',(self.id,))
                    db.execute('delete from con_gro where gid=?',(self.id,))
                    return True
            except:
                print("Failed to query.")

    def load_from_name(self):
        if self.name:
            try:
                with MySqliteDb() as db:
                    res = db.execute('select * from mygroup where name=?',(self.name,))
                    res = res.fetchone()
                    self.id = res[0]
                    self.memo = res[2]
            except:
                pass

    def all(self):
        try:
            with MySqliteDb() as db:
                res = db.execute('select * from mygroup')
                return res.fetchall()
        except:
            print("Failed to query.")

    def add_contact(self,cid):
        if cid and self.id != -1:
            try:
                with MySqliteDb() as db:
                    db.execute('insert into con_gro (cid,gid) values (?,?)',(cid,self.id))
                return True
            except:
                print('Failed to query.')

    def del_contact(self,cid):
        try:
            with MySqliteDb() as db:
                db.execute('delete from con_gro where cid=? and gid=?',(cid,self.id))
            return True
        except:
            pass

    def all_contacts(self):
        # return objects of members
        if self.id != -1:
            try:
                with MySqliteDb() as db:
                    cts = []
                    cids = db.execute('select cid from con_gro where gid=?',(self.id,))
                    cids = cids.fetchall()
                    for cid in cids:
                        c = Contact()
                        c.load(cid[0])
                        cts.append(c)
                return cts
            except:
                print('Failed to query.')

def info():
    sqls = [
        'select * from contact',
        'select * from mygroup',
        'select * from con_gro',
    ]
    try:
        with MySqliteDb() as db:
            for sql in sqls:
                res = db.execute(sql)
                res = res.fetchall()
                for r in res:
                    print(r)
    except:
        print('Failed to query.')


if __name__ == '__main__':
    cts = [
        {'name':'Lily','home_tel':'0551-98789233','memo':"anhui"},
        {'name':'Bob','office_tel':'021-94679233','memo':"shanghai"},
        {'name':'Mike','mobile_phone':'18298781089'},
        {'name':'John','home_tel':'010-57989043','memo':"beijing"},
        {'name':'Green','mobile_phone':'13908707652',},
        {'name':'Tom','mobile_phone':'13109008759'},
    ]

    gps = [
        {"name":'friends'},
        {'name':'family'},
        {'name':"students"},
    ]
    
    print('Initializing database...')
    initDb()
    print('Inserting data...')
    for ct in cts:
        Contact(**ct).save()

    for gp in gps:
        Group(**gp).save()

    print('Data queried from database')
    info()

    print('Check contact，and add it to group 2...')
    ct = Contact(name='Tom')
    ct.load_from_name()
    print(ct.id,ct.name)
    ct.add_to_group(2)

    cta = Contact()
    cta.load(1)
    print(cta.id,cta.name)
    cta.add_to_group(1)

    ctb = Contact()
    ctb.load(3)
    print(ctb.id,ctb.name)
    ctb.add_to_group(2)

    ctb = Contact()
    ctb.load(5)
    print(ctb.id,ctb.name)
    ctb.add_to_group(2)
    info()

    print("Check all contacts:")
    for r in Contact().all():
        print(r)

    print("Check all groups:")
    for r in Group().all():
        print(r)

    print("Check a member in a specific group:")
    gp = Group(name='family')
    gp.load_from_name()
    print(gp.id,gp.name)

    print("Add a member through a group:")
    gp.add_contact(4)

    cts = gp.all_contacts()
    for ct in cts:
        print(ct.id,ct.name)

    print("Delete a member in a group:")
    gp.del_contact(5)
    info()

    print("Delete contacts:")
    ct = Contact()
    ct.load(5)
    ct.delete()
    info()

    print("Delete groups:")
    gp = Group()
    gp.load(2)
    gp.delete()

    info()