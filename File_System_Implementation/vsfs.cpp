//
//  main.cpp
//  vsfs
//
//  Created by IwfWcf on 13-2-21.
//  Copyright (c) 2013年 IwfWcf. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

class bitmap
{
public:
    int size;
    vector<int> bmap;
    
    bitmap() {}
    
    bitmap(int size)
    {
        this -> size = size;
        bmap.clear();
        for (int i = 0; i < size; i++) bmap.push_back(0);
    }
    
    int alloc()
    {
        for (int i = 0; i < bmap.size(); i++)
            if (!bmap[i]) {
                bmap[i] = 1;
                return i;
            }
        return -1;
    }
    
    void free(int num)
    {
        bmap[num] = 0;
    }
    
    void markAllocated(int num)
    {
        bmap[num] = 1;
    }
};

class block
{
public:
    string ftype, data;
    int dirUsed, maxUsed;
    vector<pair<string, int>> dirList;
    
    block(string ftype)
    {
        this -> ftype = ftype;
        dirUsed = 0;
        maxUsed = 32;
        dirList.clear();
        data = "";
    }
    
    void setType(string ftype)
    {
        this -> ftype = ftype;
    }
    
    void addData(string data)
    {
        this -> data = data;
    }
    
    int getNumEntries()
    {
        return dirUsed;
    }
    
    int getFreeEntries()
    {
        return maxUsed - dirUsed;
    }
    
    pair<string, int> getEnTry(int num)
    {
        return dirList[num];
    }
    
    void addDirEntry(string name, int inum)
    {
        dirList.push_back(make_pair(name, inum));
        dirUsed++;
    }
    
    void delDirEntry(string name)
    {
        int pos = name.rfind("/");
        string dname = name.substr(pos + 1, name.size() - 1 - pos);
        for (int i = 0; i < dirList.size(); i++)
            if (dirList[i].first == dname) {
                dirList.erase(dirList.begin() + i);
                dirUsed--;
                return;
            }
    }
    
    bool dirEntryExists(string name)
    {
        for (int i = 0; i < dirList.size(); i++)
            if (name == dirList[i].first) return true;
        return false;
    }
    
    void free()
    {
        dirUsed = 0;
        data = "";
        ftype = "free";
    }
};

class inode
{
public:
    string ftype;
    int addr, refCnt;
    
    inode(string ftype = "free", int addr = -1, int refCnt = 1)
    {
        setAll(ftype, addr, refCnt);
    }
    
    void setAll(string ftype, int addr, int refCnt)
    {
        this -> ftype = ftype;
        this -> addr = addr;
        this -> refCnt = refCnt;
    }
    
    void incRefCnt()
    {
        refCnt++;
    }
    
    void decRefCnt()
    {
        refCnt--;
    }
    
    int getRefCnt()
    {
        return refCnt;
    }
    
    void setType(string ftype)
    {
        this -> ftype = ftype;
    }
    
    void setAddr(int block)
    {
        addr = block;
    }
    
    int getAddr()
    {
        return addr;
    }
    
    string getType()
    {
        return ftype;
    }
    
    void free()
    {
        ftype = "free";
        addr = -1;
    }
};

class fs
{
public:
    int numInodes, numData, ROOT, userNum;
    string currentDir;
    string username[2] = {"admin", "guest"};
    string password[2] = {"admin", "guest"};
    bitmap ibitmap, dbitmap;
    vector<inode> inodes;
    vector<block> data;
    vector<string> files, dirs;
    map<string, int> nameToInum;
    
    fs(int numInodes, int numData)
    {
        this -> numInodes = numInodes;
        this -> numData = numData;
        
        ibitmap = bitmap(numInodes);
        inodes.clear();
        for (int i = 0; i < numInodes; i++) inodes.push_back(inode());
        
        dbitmap = bitmap(numData);
        data.clear();
        for (int i = 0; i < numData; i++) data.push_back(block("free"));
        
        ROOT = 0;
        
        ibitmap.markAllocated(ROOT);
        inodes[ROOT].setAll("d", 0, 2);
        dbitmap.markAllocated(ROOT);
        data[0].setType("d");
        data[0].addDirEntry(".", ROOT);
        data[0].addDirEntry("..", ROOT);
        
        files.clear();
        dirs.clear(), dirs.push_back("/");
        nameToInum.clear(), nameToInum["/"] = ROOT;
        
        currentDir = "/";
        
        userNum = 2;
        for (int i = 0; i < userNum; i++) doCreate(username[i], "d");
    }
    
    int inodeAlloc()
    {
        return ibitmap.alloc();
    }
    
    void inodeFree(int inum)
    {
        ibitmap.free(inum);
        inodes[inum].free();
    }
    
    int dataAlloc()
    {
        return dbitmap.alloc();
    }
    
    void dataFree(int bnum)
    {
        dbitmap.free(bnum);
        data[bnum].free();
    }
    
    string getParent(string name)
    {
        int pos = name.rfind("/");
        if (pos == 0) return "/";
        return name.substr(0, pos);
    }
    
    int dir()
    {
        int inum = nameToInum[currentDir];
        int dblock = inodes[inum].getAddr();
        int n = data[dblock].dirList.size();
        printf("File Name\tFile Type\tPhysical Address\tFile Size (Bytes)\n");
        for (int i = 0; i < n; i++) {
            string filename = data[dblock].dirList[i].first;
            inum = data[dblock].dirList[i].second;
            string filetype = inodes[inum].getType();
            int address = inodes[inum].getAddr();
            int filesize;
            if (filetype == "d" || address == -1) filesize = 0;
            else filesize = data[address].data.size();
            if (filetype == "d") filetype = "<dir>";
            else filetype = "file";
            cout << filename << "\t" << filetype << "\tblock " << address << "\t" << filesize << "\n";
        }
        return 0;
    }
    
    int deleteDir(string tdir)
    {
        if (nameToInum.find(tdir) == nameToInum.end()) return -1;
        int inum = nameToInum[tdir];
        if (inodes[inum].getType() != "d") return -1;
        
        int dblock = inodes[inum].getAddr();
        if (data[dblock].dirUsed > 2) return -1;
        dataFree(dblock);
        inodeFree(inum);
        
        string parent = getParent(tdir);
        int pinum = nameToInum[parent];
        int pblock = inodes[pinum].getAddr();
        data[pblock].delDirEntry(tdir);
        
        dirs.erase(find(dirs.begin(), dirs.end(), tdir));
        return 0;
    }
    
    int deleteFile(string tfile)
    {
        if (nameToInum.find(tfile) == nameToInum.end()) return -1;
        int inum = nameToInum[tfile];
        if (inodes[inum].getType() != "f") return -1;
        
        if (inodes[inum].getRefCnt() == 1) {
            int dblock = inodes[inum].getAddr();
            dataFree(dblock);
            inodeFree(inum);
        } else
            inodes[inum].decRefCnt();
        
        string parent = getParent(tfile);
        int pinum = nameToInum[parent];
        int pblock = inodes[pinum].getAddr();
        data[pblock].delDirEntry(tfile);
        
        files.erase(find(files.begin(), files.end(), tfile));
        return 0;
    }
    
    int createLink(string target, string newfile, string parent)
    {
        int parentInum = nameToInum[parent];
        
        int pblock = inodes[parentInum].getAddr();
        if (data[pblock].getFreeEntries() <= 0) return -1;
        
        if (data[pblock].dirEntryExists(newfile)) return -1;
        
        if (nameToInum.find(target) == nameToInum.end()) return -1;
        int tinum = nameToInum[target];
        if (inodes[tinum].getType() != "f") return -1;
        inodes[tinum].incRefCnt();
        
        inodes[parentInum].incRefCnt();
        
        data[pblock].addDirEntry(newfile, tinum);
        return tinum;
    }
    
    int createFile(string parent, string newfile, string ftype)
    {
        int parentInum = nameToInum[parent];
        
        int pblock = inodes[parentInum].getAddr();
        if (data[pblock].getFreeEntries() <= 0) return -1;
        
        int block = inodes[parentInum].getAddr();
        if (data[block].dirEntryExists(newfile)) return -1;
        
        int inum = inodeAlloc();
        if (inum == -1) return -1;
        
        int fblock = dataAlloc();
        if (fblock == -1) {
            inodeFree(inum);
            return -1;
        }
        int refCnt;
        if (ftype == "d") {
            refCnt = 2;
            data[fblock].setType("d");
            data[fblock].addDirEntry(".", inum);
            data[fblock].addDirEntry("..", parentInum);
        } else
            refCnt = 1;
        
        inodes[inum].setAll(ftype, fblock, refCnt);
        
        inodes[parentInum].incRefCnt();
        
        data[pblock].addDirEntry(newfile, inum);
        return inum;
    }
    
    int readFile(string tfile)
    {
        if (nameToInum.find(tfile) == nameToInum.end()) return -1;
        int inum = nameToInum[tfile];
        if (inodes[inum].getType() != "f") return -1;
        int fblock = inodes[inum].getAddr();
        cout << "File Content:\n" << data[fblock].data << "\n";
        return 0;
    }
    
    int writeFile(string tfile, string data)
    {
        if (nameToInum.find(tfile) == nameToInum.end()) return -1;
        int inum = nameToInum[tfile];
        if (inodes[inum].getType() != "f") return -1;
        int fblock = inodes[inum].getAddr();
        this -> data[fblock].addData(data);
        return 0;
    }
    
    int doLogin(string uid, string pwd)
    {
        for (int i = 0; i < userNum; i++)
            if (uid == username[i] && pwd == password[i]) {
                currentDir = "/" + username[i];
                return 0;
            }
        return -1;
    }
    
    int doDir()
    {
        cout << "Your current path is: " << currentDir << "\n";
        return dir();
    }
    
    int doCd(string ndir)
    {
        string dir;
        if (ndir == ".") return 0;
        if (ndir == "..") {
            dir = getParent(currentDir);
            if (dir == "/") return -1;
        } else
            dir = currentDir + "/" + ndir;
        if (nameToInum.find(dir) == nameToInum.end()) return -1;
        currentDir = dir;
        return 0;
    }
    
    int doRmdir(string ndir)
    {
        string tdir = currentDir + "/" + ndir;
        return deleteDir(tdir);
    }
    
    int doLink(string target, string nfile)
    {
        string parent = currentDir, fullName;
        
        if (parent == "/") fullName = parent + nfile;
        else fullName = parent + "/" + nfile;
        
        int inum = createLink(target, nfile, parent);
        if (inum >= 0) {
            files.push_back(fullName);
            nameToInum[fullName] = inum;
            return 0;
        }
        return -1;
    }
    
    int doCreate(string nfile, string ftype)
    {
        string parent = currentDir, fullName;
        
        if (parent == "/") fullName = parent + nfile;
        else fullName = parent + "/" + nfile;
        
        int inum = createFile(parent, nfile, ftype);
        if (inum >= 0) {
            if (ftype == "d") dirs.push_back(fullName);
            else files.push_back(fullName);
            nameToInum[fullName] = inum;
            return 0;
        }
        return -1;
    }
    
    int doDelete(string nfile)
    {
        string dfile = currentDir + "/" + nfile;
        return deleteFile(dfile);
    }
    
    int doRead(string nfile)
    {
        string afile = currentDir + "/" + nfile;
        return readFile(afile);
    }
    
    int doWrite(string nfile, string data)
    {
        string afile = currentDir + "/" + nfile;
        return writeFile(afile, data);
    }
};

int iNum = 160, dNum = 56;
fs vsfs(iNum, dNum);

void help()
{
    cout << "Welcome to VSFS (the Very Simple File System), your current path is " << vsfs.currentDir << "\n";
    cout << "You can use following instructions:\n";
    cout << "login [username] [password]\n";
    cout << "//login with another user\n";
    cout << "dir\n";
    cout << "//list current directory content\n";
    cout << "cd [directory]\n";
    cout << "//change path to a directory in current directory\n";
    cout << "mkdir [directory]\n";
    cout << "//creatre a directory in current directory\n";
    cout << "rmdir [directory]\n";
    cout << "//delete an empty directory in current directory\n";
    cout << "link [target file absolute path] [link file name]\n";
    cout << "//create a hard link to target file in current directory\n";
    cout << "create [filename]\n";
    cout << "//create a file in current directory\n";
    cout << "delete [filename]\n";
    cout << "//delete a file in current directory\n";
    cout << "read [filename]\n";
    cout << "//display a file in current directory's content\n";
    cout << "write [filename] [data]\n";
    cout << "//write content into a file in current directory\n";
}

int main(int argc, const char * argv[])
{
    printf("Please login with your username and password\n");
    string username, password;
    while (1) {
        printf("username:");
        cin >> username;
        printf("password:");
        cin >> password;
        if (!vsfs.doLogin(username, password)) break;
        printf("Incorrect username or password, please try again\n");
    }
    help();
    string instruction, filename, data, directory, target;
    while (1) {
        cin >> instruction;
        if (instruction == "login") {
            while (1) {
                cin >> username >> password;
                if (!vsfs.doLogin(username, password)) break;
                printf("Incorrect username or password, please try again\n");
            }
            help();
        }
        if (instruction == "dir") vsfs.doDir();
        if (instruction == "cd") {
            cin >> directory;
            if (vsfs.doCd(directory) == -1) printf("Change path failed.\n");
            else cout << "Current path is: " << vsfs.currentDir << "\n";
        }
        if (instruction == "mkdir") {
            cin >> directory;
            if (vsfs.doCreate(directory, "d") == -1) printf("Create directory failed.\n");
            else printf("Create directory succeeded.\n");
        }
        if (instruction == "rmdir") {
            cin >> directory;
            if (vsfs.doRmdir(directory) == -1) printf("Delete directory failed.\n");
            else printf("Delete directory succeeded.\n");
        }
        if (instruction == "link") {
            cin >> target >> filename;
            if (vsfs.doLink(target, filename) == -1) printf("Create link failed.\n");
            else printf("Create link succeeded.\n");
        }
        if (instruction == "create") {
            cin >> filename;
            if (vsfs.doCreate(filename, "f") == -1) printf("Create file failed.\n");
            else printf("Create file succeeded.\n");
        }
        if (instruction == "delete") {
            cin >> filename;
            if (vsfs.doDelete(filename) == -1) printf("Delete file failed.\n");
            else printf("Delete file succeeded.\n");
        }
        if (instruction == "read") {
            cin >> filename;
            if (vsfs.doRead(filename) == -1) printf("Read file failed.\n");
        }
        if (instruction == "write") {
            cin >> filename >> data;
            if (vsfs.doWrite(filename, data) == -1) printf("Write file failed.\n");
            else printf("Write file succeeded.\n");
        }
    }
    return 0;
}

