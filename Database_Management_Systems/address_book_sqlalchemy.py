from sqlalchemy import create_engine,String,Integer,Column,ForeignKey,Table,Text,Sequence
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker,relationship

db_name = 'mysql+mysqlconnector://root:admin@localhost:3306/test'

Base = declarative_base()

con_gro = Table('asso',Base.metadata,
                Column('con_id',Integer,ForeignKey('contact.id')),
                Column('gro_id',Integer,ForeignKey('mygroup.id')))

class Contact(Base):
    __tablename__ = 'contact'
    id =  Column(Integer,Sequence('contact_id'),primary_key=True)
    name = Column(String(20))
    home_tel = Column(String(20))
    office_tel = Column(String(20))
    mobile_phone = Column(String(20))
    memo = Column(Text)
    groups = relationship("Group",secondary=con_gro,backref="contacts")

class Group(Base):
    __tablename__ = 'mygroup'
    id =  Column(Integer,Sequence('mygroup_id'),primary_key=True)
    name = Column(String(20))
    memo = Column(Text)

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


    engine = create_engine(db_name)
    Session = sessionmaker(bind=engine)
    session = Session()

    print('Initializing database...')
    Base.metadata.drop_all(engine)
    Base.metadata.create_all(engine)

    print('Inserting data...')
    for ct in cts:
        session.add(Contact(**ct))
    for gp in gps:
        session.add(Group(**gp))

    session.commit()

    print('Data queried from database')
    for c in session.query(Contact).all():
        print(c.id,c.name,c.home_tel,
            c.office_tel,c.mobile_phone,c.memo)
    for g in session.query(Group).all():
        print(g.id,g.name,g.memo)

    print('Check contact，and add it to group 2...')
    ct = session.query(Contact).filter_by(name="Tom").first()
    print(ct.id,ct.name)
    gp = session.query(Group).filter_by(id=2).first()
    gp.contacts.append(ct)

    ct = session.query(Contact).filter_by(id=1).first()
    print(ct.id,ct.name)
    gpo = session.query(Group).filter_by(id=1).first()
    gpo.contacts.append(ct)

    ct = session.query(Contact).filter_by(id=3).first()
    print(ct.id,ct.name)
    gp.contacts.append(ct)

    ct = session.query(Contact).filter_by(id=5).first()
    print(ct.id,ct.name)
    gp.contacts.append(ct)

    session.commit()

    print("Check a member in a specific group:")
    gp = session.query(Group).filter_by(name="family").first()
    print(gp.id,gp.name)
    for c in gp.contacts:
        print(c.id,c.name)

    print("Delete a member in a group:")
    gp.contacts.remove(c)
    session.commit()
    for c in gp.contacts:
        print(c.id,c.name)

    print("Delete contacts:")
    c = session.query(Contact).filter_by(id=5).first()
    session.delete(c)

    print("Delete groups:")
    g = session.query(Group).filter_by(id=2).first()
    session.delete(g)

    session.commit()

    for c in session.query(Contact).all():
        print(c.id,c.name,c.home_tel,
            c.office_tel,c.mobile_phone,c.memo)
    for g in session.query(Group).all():
        print(g.id,g.name,g.memo)