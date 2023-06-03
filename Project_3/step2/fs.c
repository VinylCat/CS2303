#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
/*
128 INODEs 128 Datablock
162 BLOCKS in all.

BLOCKSIZE 256 bytes
Block0 is for Superblock
BLOCK1 is for Map 
BLOCK2- BLOCK33 are for 64 Inodes
BLOCK34 - BLOCK161 are for 128 Datablocks
*/
typedef uint32_t _u32;
typedef uint8_t _u8;

#define MAX_COMMANDS 256
#define BLOCKSIZE 256
#define INODESIZE 64
#define BLOCKNUM 162
#define BLOCKBASE 34
#define INODEBASE 2
#define DATASIZE 1024

typedef struct
{
    _u32 s_inodes_count;
    _u32 s_blocks_count;
    _u32 s_free_inodes_count;
    _u32 s_free_blocks_count;
    _u32 s_root;

}SUPERBLOCK;

typedef struct  //Inode 64 bytes
{
    _u32 i_id;//Inode ID
    _u32 i_mode;//directorty file or file 1 for directory 0 for file
    _u32 i_uid;//num of file
    _u32 i_gid;//father directory, record the inode
    _u32 i_size;
    _u32 i_access;
    _u32 i_blocknums;
    _u32 i_ctime;//create time;
    _u32 i_mtime;//modified time;
    _u32 i_atime;//access time;
    _u32 i_direct[4];
    _u32 i_sindirect;
    _u32 i_dindirect;//son directory 32  is max number
}INODE;

typedef struct //64 bits for INODE 128 bits for BLOCKS Totaoly 24 bytes
{
    _u32 inodeMap[2];
    _u32 blockMap[4];
}BITMAP;

typedef struct //28 bytes in all. So a block can hold 8 files inall.
{
    _u32 f_inodeNum;
    char f_fileName[24];
}FILEDIRECTORY;

typedef struct //256 bytes ; just want it to point to 8 blocks
{
    _u32 s_blocknum[8];
    _u32 available[8];
}SINGLEINDIRECT;

typedef struct //136 - 256 bytes, stored in i_dindirect in directory file, for store son directory.
{
    _u32 sonDirectory[32];// 128 bytes
    _u32 map;
    _u32 num;
}SONDIRECTOPRYTABLE;


SUPERBLOCK sb;
BITMAP bmp;
INODE currentDirectory;
_u32 currentDirectoryInode;
char* diskfile;

void delchar(char* str, const size_t max_len)
{
	char strtmp[max_len];
	const size_t nlen = strlen(str);
	size_t ptr = 0;
	for (size_t i = 0; i < nlen; i++)
		if (str[i] != 10 && str[i] != 13)
			strtmp[ptr++] = str[i];
	for (size_t i = 0; i < ptr; i++)
		str[i] = strtmp[i];
	str[ptr] = 0;
}

void init_SONDIRECTORYTABLE(int blockPos)
{
    SONDIRECTOPRYTABLE stb;
    for (int i =0; i< 32; i++)
        stb.sonDirectory[i] = -1;
    stb.map = 0;
    stb.num = 0;
    
    void *buf = &stb;
    memcpy(&diskfile[(BLOCKBASE + blockPos) * BLOCKSIZE], buf, BLOCKSIZE);
}

void init_SUPERBLOCK()
{
    sb.s_inodes_count = 64;
    sb.s_blocks_count = 128;
    sb.s_free_inodes_count = 634;
    sb.s_free_blocks_count = 124;
    sb.s_root = 0;
}

void init_SINGLEINDIRECT(int blockPos)//for the available, 0 for not exist, 1 for can be used , -1 for full
{
    SINGLEINDIRECT sid;
    for (int i = 0; i < 8; i++)
    {
        sid.s_blocknum[i] = -1;
        sid.available[i] = 0;
    }

    char *buf = (char*) &sid;
    memcpy(&diskfile[(BLOCKBASE + blockPos)*BLOCKSIZE], buf, 256);
}   

#define BITS_PER_WORD 32
#define MASK 0x1f
#define SHIFT 5
//type0 used for inode, type1 used for block
void setBitMap(int type, int N)
{
    if (type == 0)
    {
        //printf("Inodemap[] = %d\n", bmp.inodeMap[0]);
        bmp.inodeMap[N >> SHIFT] |= 1 << (N & 0x07);
        sb.s_free_inodes_count--;
        //printf("Inodemap[] = %d, and free is %u.\n", bmp.inodeMap[0], sb.s_free_inodes_count);
    }
    if (type == 1)
    {    
        //printf("blockmap[] = %d\n", bmp.blockMap[0]);
        bmp.blockMap[N >> SHIFT] |= 1 << (N & 0x07);
        sb.s_free_blocks_count--;
        //printf("blockmap[] = %d\n", bmp.blockMap[0]);
    }
}

void init_BITMAP()
{
    bmp.inodeMap[0] = 0;
    bmp.inodeMap[1] = 0;
    for (int i = 0; i < 4; i++)
        bmp.blockMap[i] = 0;
}

void init_INODE(INODE *i)
{
    time_t currentTime;
    currentTime = time(NULL);
    i->i_id = -1;
    i->i_mode = -1;// 0 for file 1 for directory
    i->i_uid = -1;// num of the file in the directory
    i->i_gid = -1;//father directory, record the inode 
    i->i_size = 0;
    i->i_access = -1;
    i->i_blocknums = -1;
    i->i_ctime = (_u32)currentTime;
    i->i_mtime = (_u32)currentTime;
    i->i_atime = (_u32)currentTime;
    for (int k = 0; k < 4; k++)
        i->i_direct[k] = -1;
    i->i_sindirect = -1;
    i->i_dindirect = -1;//son directory 32 is max number
}

//c for create a for access m for modify
void timeUpdate(INODE *i, int c, int a, int m)
{
    time_t currentTime;
    currentTime = time(NULL);
    if (c)
        i->i_ctime = (_u32)currentTime;
    if (a)
        i->i_atime = (_u32)currentTime;
    if (m)
        i->i_mtime = (_u32)currentTime;
    return;    
}

//type0 used for inode, type 1 used for block
int searchBitMap(int type)
{
    if (type == 0)
    {
        for (int i = 0; i < 2; i++)
        {
            for (int bitpos = 0; bitpos < 32; bitpos++)
            {
                _u32 mask = 1 << bitpos;
                if ((bmp.inodeMap[i] & mask) == 0 )
                    return bitpos + i * 32;
            }
        }
    }
    else if (type == 1)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int bitpos = 0; bitpos < 32; bitpos++)
            {
                _u32 mask = 1 << bitpos;
                if ((bmp.blockMap[i] & mask) == 0)
                    return bitpos + i * 32;
            }
        }
    }
    else
        return -1;
}

//type 0 for inode, type 1 for block
void returnBitMap(int type, int N)
{
    if (type == 0)
    {
        bmp.inodeMap[N >> SHIFT] |= 0 << (N & 0x07);
        sb.s_free_inodes_count++;
        memset(&diskfile[INODEBASE * BLOCKSIZE + N * INODESIZE], 0, INODESIZE);
    }
    if (type == 1)
    {
        bmp.blockMap[N >> SHIFT ] |= 0 << (N & 0x07);
        sb.s_free_blocks_count++;
        memset(&diskfile[(BLOCKBASE + N) * BLOCKSIZE], 0, BLOCKSIZE);       
    }
}

int addFiletoDirectory(int fileInode, char* fileName)
{
    //a block can hold 8 files, can hold 96 files in all; 4*8 + 8*8
    //printf("id: %u, uid: %u.\n",currentDirectory.i_id, currentDirectory.i_uid);
    
    if (currentDirectory.i_uid >= 96)
    {
        printf("Error: current directory is full.\n");
        return -1;
    }
    FILEDIRECTORY cd;
    
    if (strlen(fileName) >= 24)
    {
        printf("Error: the file name is too long.\n");
        return -1;
    }
    strcpy(cd.f_fileName, fileName);
    cd.f_inodeNum = fileInode;
    char* buf = (char*) &cd;
    //memcpy(&diskfile[0], buf, sizeof(sb));
    if (currentDirectory.i_uid <= 7)//can store in the first block;
    {

        //blocknum to be written :currentDirectory.i_direct[0]
        memcpy(&diskfile[ (BLOCKBASE + currentDirectory.i_direct[0])*BLOCKSIZE + currentDirectory.i_uid * 32], buf, sizeof(cd));
        currentDirectory.i_size += sizeof(cd);
        //printf("in addfiledirectory %u id: %u, name: %s.\n",currentDirectory.i_uid, cd.f_inodeNum, cd.f_fileName);
    }
    else if (currentDirectory.i_uid > 7 && currentDirectory.i_uid <= 31)//store in other 3 direct block
    {
        
        _u32 directnum = (currentDirectory.i_uid) / 8;
        _u32 offset = (currentDirectory.i_uid) % 8;
        if (currentDirectory.i_direct[directnum] == -1) //case: the block isn't be taken
        {
            int blockpos = searchBitMap(1);
            if (blockpos == -1)
            {
                printf("Error: failed to find the block.\n");
                return -1;
            }
            setBitMap(1, blockpos);
            currentDirectory.i_direct[directnum] = blockpos;
            currentDirectory.i_blocknums++ ;
        }    
        timeUpdate(&currentDirectory, 0, 1, 1);
        memcpy(&diskfile[ (BLOCKBASE + currentDirectory.i_direct[directnum])*BLOCKSIZE + offset * 32], buf, sizeof(cd));
        currentDirectory.i_size += sizeof(cd);
        //printf("here %u, %u\n", directnum, offset);

    }
    else //store in sindirect
    {
            if(currentDirectory.i_sindirect == -1) // not exist
            {
                int blockpos = searchBitMap(1);
                if (blockpos == -1)
                {
                    printf("Error: failed to find the block.\n");
                  return -1;
                }
                setBitMap(1, blockpos);
                currentDirectory.i_sindirect = blockpos;
                init_SINGLEINDIRECT(blockpos);
            }
            timeUpdate(&currentDirectory, 0, 1, 1);
            SINGLEINDIRECT sid;// dont forget to write back
            memcpy(&sid, &diskfile[(BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], sizeof(sid));
            int idm = (currentDirectory.i_uid - 32)/8;
            int offset = (currentDirectory.i_uid - 32)%8;
            // when need to delete, the hole block arrangement of the Inode need to adjust!
            if(sid.s_blocknum[idm] == -1)// not exist
            {
                int blockpos = searchBitMap(1);
                setBitMap(1, blockpos); 
                if (blockpos == -1)
                {
                    printf("Error: failed to find the block.\n");
                    return -1;
                }
                sid.s_blocknum[idm] = blockpos;
                char *buf = (char*) &sid;
                memcpy(&diskfile[(BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], buf, 256);
            }
            
            memcpy(&diskfile[(BLOCKBASE + sid.s_blocknum[idm]) * BLOCKSIZE + offset * 32] , buf, sizeof(cd));
            currentDirectory.i_size += sizeof(cd);
    }
    currentDirectory.i_uid ++;
    return 0;   
}

void init_FS(char* diskfile)
{
    init_SUPERBLOCK();
    char* buf = (char*) &sb;
    memcpy(&diskfile[0], buf, sizeof(sb));
    //create bmp

    init_BITMAP();
    //create \root
    //BitMap for \root
    setBitMap(0, 0);
    setBitMap(1, 0);
    //inode for \root
    INODE root;
    init_INODE(&root);
    root.i_id = 0;
    root.i_mode = 1;
    root.i_access = 777;
    root.i_gid = -1;// it is the root
    root.i_uid = 0; // num of file
    root.i_size = 0;
    root.i_blocknums = 1;
    root.i_direct[0] = 0;
    root.i_dindirect = -1;// subdirectory
    buf = (char*) &root;
    //save Inode!!!
    memcpy(&diskfile[INODEBASE * BLOCKSIZE], buf, INODESIZE);
    currentDirectoryInode = 0;
    memcpy(&currentDirectory, &diskfile[INODEBASE * BLOCKSIZE], sizeof(currentDirectory));
    char name[]= "root";

    addFiletoDirectory(currentDirectoryInode, name);    
}

int createFile(char* fileName) //-1 for error
{
    
    //search bitmap

    int inodePos, blockPos;//o based
    inodePos = searchBitMap(0);
    blockPos = searchBitMap(1);
    if (blockPos == -1 | inodePos == -1)
        return -1;
    
    //printf("INode:%d\n", inodePos);
    //printf("block:%d\n", blockPos);
    //take inode inode--
    //take block block--
    setBitMap(0, inodePos);
    setBitMap(1, blockPos);

    INODE newfile;
    init_INODE(&newfile);
    newfile.i_id = inodePos;
    newfile.i_mode = 0;
    newfile.i_access = 777;
    newfile.i_uid = newfile.i_gid = 1;
    newfile.i_size = 0;;
    newfile.i_direct[0] = blockPos;
    newfile.i_blocknums = 1;
    //write it to the directory
    timeUpdate(&newfile,1,1,1);
    char *buf = (char*) &newfile;
    memcpy(&diskfile[(INODEBASE * BLOCKSIZE + inodePos * INODESIZE)], buf, INODESIZE);

    int res = addFiletoDirectory(inodePos, fileName);
    if (res != 0)
        return -1;
    else 
        return 0;
    
}

int fatherLink (int sonInode)// 1 for error
{
    if (currentDirectory.i_dindirect == -1)//creat list for subdirectory
    {
        
        int blockPos = searchBitMap(1);
        if (blockPos == -1)
        {
            printf("Error: not enough space.\n");
            return 1;
        }
        setBitMap(1, blockPos);
        currentDirectory.i_dindirect = blockPos;
        init_SONDIRECTORYTABLE(blockPos);
        
    }
    timeUpdate(&currentDirectory, 0, 1, 1);
    
    //find the available block,
    //not like the condition in file, directory has no requirement of the sequence of the directory
    SONDIRECTOPRYTABLE stb; // don't forget to write back
    memcpy(&stb, &diskfile[(BLOCKBASE + currentDirectory.i_dindirect) * BLOCKSIZE], sizeof(stb));
    
    if (stb.num == 0) // no exist;
    {
        // simple condition, just store in the first sonDirectory[0], and update the map
        stb.num ++;
        stb.sonDirectory[0] = sonInode;
        stb.map |= 1 << 0;
        
    }
    else if (stb.num >= 32) // full
    {
        printf("Error: not enough space.\n");
        return 1;
    }
    else
    {
        stb.num ++ ;
        int availablePos;
        for (availablePos = 0; availablePos < 32; availablePos++)
        {
            _u32 mask = 1 << availablePos;
            if (((stb.map & mask) == 0))
                break;
        }
        stb.sonDirectory[availablePos] = sonInode;
        stb.map |= 1 << availablePos;
    }
    //  write back
    
    char *buf = (char*) &stb;
    memcpy(&diskfile[(BLOCKBASE + currentDirectory.i_dindirect) * BLOCKSIZE], buf, BLOCKSIZE);
    return 0;
}

int createDirectory(char* directoryName) //1 for error
{
    int inodePos, blockPos;
    inodePos = searchBitMap(0);
    blockPos = searchBitMap(1);
    
    if (blockPos == -1 | inodePos == -1)
        return -1;
    setBitMap(0, inodePos);
    setBitMap(1, blockPos);
    
    INODE newdirect;
    init_INODE(&newdirect);
    newdirect.i_id = inodePos;
    newdirect.i_mode = 1;
    newdirect.i_uid = 0;
    //take the father directory
    newdirect.i_gid = currentDirectoryInode;
    newdirect.i_size = 0;
    newdirect.i_access = 777;
    newdirect.i_blocknums = 1;
    newdirect.i_direct[0] = blockPos;
    char *buf = (char*)&newdirect;
    memcpy(&diskfile[INODEBASE * BLOCKSIZE + inodePos * INODESIZE], buf, INODESIZE);
    _u32 tmpInode = currentDirectoryInode;
    memcpy(&diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE], &currentDirectory, INODESIZE);
    
    currentDirectoryInode = newdirect.i_id;
    memcpy(&currentDirectory, &diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE], INODESIZE);//update
    //printf("uid:%u.\n", currentDirectory.i_uid);
    addFiletoDirectory(inodePos, directoryName);//record the name
    memcpy(&diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE], &currentDirectory, INODESIZE);

    currentDirectoryInode = tmpInode;
    memcpy(&currentDirectory, &diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE], INODESIZE);

    //buf = (char*) &newdirect;
    //memcpy(&diskfile[INODEBASE * BLOCKSIZE + inodePos * INODESIZE], buf, INODESIZE);
    //in father directory we should do
    
    int res;
    res = fatherLink(inodePos);


    if (res != 0)
        return 1;
    else 
        return 0;
}

//to delete the file, file should in the current directory
int deleteFile(char * deleteFile)
{
    if (strlen(deleteFile) >= 24)
    {
        printf("Error: the file name is too long.\n");
        return -1;
    }
    FILEDIRECTORY fd;
    //printf("the directory have %u files.\n", currentDirectory.i_uid);
    int loc = 0;
    for (loc = 0; loc < currentDirectory.i_uid; loc++) //first block hold 8, from 31 to i_uid hold the rest
    {
        //find the loc
        if (loc < 32)
        {
            memcpy(&fd, &diskfile[(BLOCKBASE + currentDirectory.i_direct[loc/8]) * BLOCKSIZE + 32 * (loc % 8)], sizeof(fd));
            //printf("Now detecting: %u %s, the loc is %d.\n", fd.f_inodeNum,fd.f_fileName,loc);
            if (strcmp(fd.f_fileName, deleteFile) == 0)
                break;
        }
        else
        {
            SINGLEINDIRECT sid;
            memcpy(&sid, &diskfile[(BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], sizeof(sid));
            int offset = (loc - 31) / 8;
            memcpy(&fd, &diskfile[(BLOCKBASE + sid.s_blocknum[offset]) * BLOCKSIZE + (loc % 8) * 32], sizeof(fd));
            //printf("Now detecting: %s.\n", fd.f_fileName);
            if (strcmp(fd.f_fileName, deleteFile) == 0)
                break;
        }  
    }
    if (loc == currentDirectory.i_uid)
    {
        printf("Error: No such file exists.\n");
        return 1;
    }
    //return block: direct , singleindirect
    INODE tarI;
    memcpy(&tarI, &diskfile[INODEBASE * BLOCKSIZE + fd.f_inodeNum * INODESIZE], INODESIZE);
    for (int i = 0; i < 4; i++)
    {
        if (tarI.i_direct[i] != -1)
        {
            returnBitMap(1, tarI.i_direct[i]);
            tarI.i_direct[i] = -1;
            
        }
    }
    if (tarI.i_sindirect != -1)
    {
        SINGLEINDIRECT sid;
        memcpy(&sid, &diskfile[(BLOCKBASE + tarI.i_sindirect) * BLOCKSIZE], sizeof(sid));
        for (int j = 0; j < 8; j++)
        {
            if (sid.s_blocknum[j] != -1)
            {
                returnBitMap(1, sid.s_blocknum[j]);
                sid.s_blocknum[j] = -1;
                
            }
        }
        char* buf = (char*) &sid;
        memcpy(&diskfile[(BLOCKBASE + tarI.i_sindirect) * BLOCKSIZE], buf, BLOCKSIZE);
        
        returnBitMap(1, tarI.i_sindirect);
        tarI.i_sindirect = -1;   
        
    }
    //return inode
    returnBitMap(0, fd.f_inodeNum);
    
    //rearrange them in directory, first level in direct[i], stb
    int reLoc = loc + 1;
    int tarLoc = loc + 0;
    for (reLoc; reLoc < currentDirectory.i_uid; reLoc ++)
    {
        //only responsible for copy
        if (reLoc < 31)
        {
            FILEDIRECTORY tmp;
            memcpy(&tmp,&diskfile[(BLOCKBASE + currentDirectory.i_direct[tarLoc/8]) * BLOCKSIZE + (tarLoc % 8 ) * 32], sizeof(FILEDIRECTORY));
            memcpy(&diskfile[(BLOCKBASE + currentDirectory.i_direct[tarLoc/8]) * BLOCKSIZE + (tarLoc % 8 ) * 32],
             &diskfile[(BLOCKBASE + currentDirectory.i_direct[reLoc / 8]) * BLOCKSIZE + (reLoc % 8) * 32] , sizeof(FILEDIRECTORY));
            memset(&diskfile[(BLOCKBASE + currentDirectory.i_direct[reLoc / 8]) * BLOCKSIZE + (reLoc % 8) * 32] , 0, sizeof(FILEDIRECTORY));
            memcpy(&tmp,&diskfile[(BLOCKBASE + currentDirectory.i_direct[tarLoc/8]) * BLOCKSIZE + (tarLoc % 8 ) * 32], sizeof(FILEDIRECTORY));
            //printf("inode before: %u.\n", tmp.f_inodeNum);
        }
        else if (reLoc == 32 && tarLoc == 31)// reloc strored in sindirct(block) --> sid --> blocknum[0] --> block
        {
            SINGLEINDIRECT sid;
            memcpy(&sid, &diskfile[(BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], sizeof(sid));
            memcpy(&diskfile[(BLOCKBASE + currentDirectory.i_direct[tarLoc/8]) * BLOCKSIZE + (tarLoc % 8 ) * 32],
            &diskfile[(BLOCKBASE + sid.s_blocknum[0]) * BLOCKBASE + 0 * 32], sizeof(FILEDIRECTORY));
            memset(&diskfile[(BLOCKBASE + sid.s_blocknum[0]) * BLOCKSIZE + 0 * 32], 0, sizeof(FILEDIRECTORY));
        }
        else
        {
            SINGLEINDIRECT sid;
            memcpy(&sid, &diskfile[(BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], sizeof(sid));
            memcpy(&diskfile[(BLOCKBASE + sid.s_blocknum[(tarLoc - 32) / 8]) * BLOCKSIZE + ((tarLoc - 32) % 8) * 32], 
            &diskfile[(BLOCKBASE + sid.s_blocknum[(reLoc -32) / 8]) * BLOCKSIZE + ((reLoc - 32) % 8) * 32], sizeof(FILEDIRECTORY));
            memset(&diskfile[(BLOCKBASE + sid.s_blocknum[(reLoc -32) / 8]) * BLOCKSIZE + ((reLoc - 32) % 8) * 32],0 , sizeof(FILEDIRECTORY));

        }
        tarLoc ++;
    }
    //now delete inode of currentdirectory  uid = 9, 17, 25, 33(delet sid &[0]),41(delete sid.blocknum[1]), 49([2])
    if (currentDirectory.i_uid == 9 || currentDirectory.i_uid ==17 || currentDirectory.i_uid == 25)
    {
        returnBitMap(1, currentDirectory.i_direct[currentDirectory.i_uid/ 8]);
        currentDirectory.i_direct[currentDirectory.i_uid / 8] = -1;
    }
    if ( currentDirectory.i_uid == 33)
    {
        SINGLEINDIRECT sid;
        memcpy(&sid, &diskfile[ ( BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], sizeof(sid));
        returnBitMap(1, sid.s_blocknum[0]);
        returnBitMap(1, currentDirectory.i_sindirect);// as the sid is deleted, there is no need to write back sid and maintain the value in it
    }
    if (currentDirectory.i_uid == 41 || currentDirectory.i_uid == 49 || currentDirectory.i_uid == 57 || currentDirectory.i_uid == 65 
    || currentDirectory.i_uid == 73 || currentDirectory.i_uid == 81 || currentDirectory.i_uid == 89)
    {
        SINGLEINDIRECT sid;
        memcpy(&sid, &diskfile[ ( BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], sizeof(sid));
        returnBitMap(1, sid.s_blocknum[(currentDirectory.i_uid - 32)/8]);
        sid.s_blocknum[(currentDirectory.i_uid - 32) / 8] = -1;
        char* buf = (char*) &sid;
        memcpy(&diskfile[( BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], buf, sizeof(sid));
    }
    //dont forget uid --
    currentDirectory.i_uid --;
    return 0;
}

int searchInDir(char* name)
{
    if (currentDirectory.i_dindirect  == -1)
        return -1;
    SONDIRECTOPRYTABLE stb;
    memcpy(&stb, &diskfile[(BLOCKBASE + currentDirectory.i_dindirect) * BLOCKSIZE], sizeof (stb));
    _u32 sdInode = -1;
    if (stb.num == 0)
        return -1;
    else
    {
        for (int i = 0; i < stb.num || i < 32; i++)
        {   
            //printf("the %d th turn\n", i);
            sdInode = stb.sonDirectory[i];
            if (sdInode == -1)
                continue;
            INODE sd;
            memcpy(&sd, &diskfile[INODEBASE * BLOCKSIZE + sdInode * INODESIZE], sizeof(sd));
            //printf("sd.i_id: %u\n", sd.i_id);
            FILEDIRECTORY sdName;
            memcpy(&sdName, &diskfile[(BLOCKBASE + sd.i_direct[0]) * BLOCKSIZE], sizeof(sdName));
            //printf ("compare %s, %s.\n",name, sdName.f_fileName);
            if (strcmp(name, sdName.f_fileName) == 0)
            {
                //printf("name searched: %s, order: %d\n", sdName.f_fileName,i);
                return sdInode;
            }
        }
    }
    return -1;
    
}

int deleteDir(char* name)
{
    //search the dir 
    int result = searchInDir(name);
    if (result == -1) 
    {
        printf("Error: no directory searched.\n");
        return -1;
    }

    //check the file in the dir and delete them
    ////first should change the currentdirectory
    char* buf = (char*) &currentDirectory;
    memcpy(&diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode *INODESIZE], buf, INODESIZE);
    _u32 oriInode = currentDirectoryInode;
    currentDirectoryInode = result;
    memcpy(&currentDirectory, &diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE], INODESIZE);
    //currentdirectory is deleted directory

    FILEDIRECTORY fd;
    _u32 fileNums = currentDirectory.i_uid;//1 for 0 files, 2 for 1 files
    for(fileNums; fileNums > 1; fileNums--)
    {
        memcpy(&fd, &diskfile[(BLOCKBASE + currentDirectory.i_direct[0]) * BLOCKSIZE + 32], sizeof(fd));
        //printf("now the first file name is %s.\n",fd.f_fileName);
        deleteFile(fd.f_fileName);
    }
    
    
    //check the sonDir and delete them, use the delete deleteDir recursively
    if (currentDirectory.i_dindirect != -1)
    {
        SONDIRECTOPRYTABLE stb;
        memcpy(&stb, &diskfile[(BLOCKBASE + currentDirectory.i_dindirect) * BLOCKSIZE], sizeof(stb));
        _u32 delNum = 0;
        for (int i = 0; i < 32 || delNum < stb.num; i++)
        {
            if (stb.sonDirectory[i] != -1)//delete sondir
            {
                //get the name 
                INODE sonDir;
                memcpy(&sonDir, &diskfile[INODEBASE * BLOCKSIZE + stb.sonDirectory[i] * INODESIZE], INODESIZE);
                FILEDIRECTORY sfd;
                memcpy(&sfd, &diskfile[(BLOCKBASE + sonDir.i_direct[0]) * BLOCKSIZE], sizeof(sfd));
                int ans = deleteDir(sfd.f_fileName);
                if (ans != 0)
                {
                    printf("Error: delete sonDir.\n");
                    return 1;
                }
                delNum++;
                stb.map |= 0 << i;
            }
        }
        buf = (char*) &stb;
        memcpy(&diskfile[(BLOCKBASE + currentDirectory.i_dindirect) * BLOCKSIZE], buf, sizeof(stb));
        returnBitMap(1,currentDirectory.i_dindirect);
        currentDirectory.i_dindirect = -1;
    }
    //delete itself 
    //release inode
    //release idirect
    for (int i = 0; i < 4; i++)
    {
        if (currentDirectory.i_direct[i] != -1)
        {
            returnBitMap(1, currentDirectory.i_direct[i]);
        }
    }
    if (currentDirectory.i_sindirect != -1)
    {
        SINGLEINDIRECT sid;
        memcpy(&sid, &diskfile[(BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], sizeof(sid));
        for (int k = 0; k < 8; k++)
        {
            if(sid.s_blocknum[k] != -1)
            {
                returnBitMap(1,sid.s_blocknum[k]);
                sid.s_blocknum[k] = -1;
            }
        }
        memcpy(&diskfile[(BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], &sid, sizeof(sid));
        returnBitMap(1, currentDirectory.i_sindirect);
        currentDirectory.i_sindirect = -1;
    }
    //release direct
    //release isdirect
    returnBitMap(0,currentDirectory.i_id);
    //change father's stb;
    buf = (char*)&currentDirectory;
    memcpy(&diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE], buf, INODESIZE);

    currentDirectoryInode = oriInode;
    memcpy(&currentDirectory, &diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE], INODESIZE);

    SONDIRECTOPRYTABLE stb;
    memcpy(&stb, &diskfile[(BLOCKBASE + currentDirectory.i_dindirect) * BLOCKSIZE], sizeof (stb));
    _u32 sdInode = -1;
    int i = 0;
    for (i; i < stb.num || i < 32; i++)
       {   
        
        sdInode = stb.sonDirectory[i];
        if (sdInode == result)
        {

            stb.sonDirectory[i] = -1;
            stb.map |= 0 << i;
            stb.num--;
            break;
        }
    }
    memcpy(&diskfile[(BLOCKBASE + currentDirectory.i_dindirect) * BLOCKSIZE], &stb, sizeof (stb));
    if (i >= 32 || i >= stb.num + 1)
        return 1;
    return 0;

}

int listing(FILE* fp)
{
    FILEDIRECTORY fd;
    INODE file;
    memcpy(&fd, &diskfile[(BLOCKBASE + currentDirectory.i_direct[0]) * BLOCKSIZE], sizeof(fd));
    printf("Current directory:\n%s\n", fd.f_fileName);
    //printf("uid: %u\n", currentDirectory.i_uid);
    printf("Files:\n");
    fprintf(fp,"Yes ");
    for (int i = 1; i < currentDirectory.i_uid; i++)
    {
        if (i < 32)
        {
            memcpy(&fd, &diskfile[(BLOCKBASE + currentDirectory.i_direct[i/8]) * BLOCKSIZE + (i%8) * 32], sizeof(fd));
            memcpy(&file, &diskfile[(INODEBASE * BLOCKSIZE + fd.f_inodeNum * INODESIZE)], INODESIZE);
            printf("%s filesize: %u\n", fd.f_fileName, file.i_size);    
            time_t currentTime = (time_t)file.i_atime;
            struct tm* timeinfo = localtime(&currentTime);
            char time_str[20]; // 适当大小的字符数组来容纳格式化后的时间字符串
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
            printf("Access Time：%s\n", time_str);
            currentTime = (time_t)file.i_ctime;
            timeinfo = localtime(&currentTime);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
            printf("Create Time：%s\n", time_str);
            currentTime = (time_t)file.i_mtime;
            timeinfo = localtime(&currentTime);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
            printf("Modify Time：%s\n", time_str);
            fprintf(fp,"%s ",fd.f_fileName);
        }
        else 
        {
            SINGLEINDIRECT sid;
            memcpy(&sid, &diskfile[(BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], sizeof(sid));
            memcpy(&fd, &diskfile[(BLOCKBASE + sid.s_blocknum[(i - 32)/8]) * BLOCKSIZE + ((i-32)%8) * 32], sizeof(fd));
            printf("%s\n", fd.f_fileName);
        }
    }
    printf("SonDirectory:\n");
    if (currentDirectory.i_dindirect != -1)
    {
        SONDIRECTOPRYTABLE stb;
        memcpy(&stb, &diskfile[(BLOCKBASE + currentDirectory.i_dindirect) * BLOCKSIZE], sizeof(stb));
        int i = 0;
        int tmp = 0;
        fprintf(fp,"&");
        while (tmp < stb.num)
        {
            if (stb.sonDirectory[i] != -1)
            {
                INODE sd;
                memcpy(&sd, &diskfile[(INODEBASE * BLOCKSIZE + stb.sonDirectory[i] * INODESIZE)], INODESIZE);
                FILEDIRECTORY cd;
                memcpy(&fd, &diskfile[(BLOCKBASE + sd.i_direct[0]) * BLOCKSIZE], sizeof(cd));
                printf("%s\n", fd.f_fileName);
                fprintf(fp,"%s ",fd.f_fileName);
                tmp++;
            }
            i++;
        }
    }
    fprintf(fp, "\n");
    return 0;
}

int searchFile(char* name)
{
    for (int i = 1; i < currentDirectory.i_uid; i++)
    {
        FILEDIRECTORY fd;
        if (i < 32)
        {
            memcpy(&fd, &diskfile[(BLOCKBASE + currentDirectory.i_direct[i/8]) * BLOCKSIZE + (i%8) * 32], sizeof(fd));
            if (strcmp(fd.f_fileName, name) == 0)
                return fd.f_inodeNum;
        }
        else 
        {
            SINGLEINDIRECT sid;
            memcpy(&sid, &diskfile[(BLOCKBASE + currentDirectory.i_sindirect) * BLOCKSIZE], sizeof(sid));
            if (strcmp(fd.f_fileName, name) == 0)
                return fd.f_inodeNum;
        }
    }
    return -1;
}

int catFile(char* name, FILE* fp)
{
    int result = searchFile(name);
    if (result == -1) 
    {
        printf("Error: no file searched.\n");
        return -1;
    }
    INODE tar;
    memcpy(&tar, &diskfile[INODEBASE * BLOCKSIZE + result * INODESIZE], INODESIZE);
    timeUpdate(&tar,0, 0, 1);
    printf("Reading %s ...\nThe size is %u\nData is printed below:\n", name, tar.i_size);
    fprintf(fp, "Yes ");
    if (tar.i_size <= 0)
    {
        return 0;
    }
    else
    {
        char dataBuf[256] = {0};
        _u32 restSize =  tar.i_size;
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + tar.i_direct[0]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + tar.i_direct[0]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + tar.i_direct[1]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + tar.i_direct[1]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + tar.i_direct[2]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + tar.i_direct[2]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + tar.i_direct[3]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + tar.i_direct[3]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        //direct is running up
        if (restSize <= 0)
            return 0;
        //take sidirect
        SINGLEINDIRECT sid;
        memcpy(&sid, &diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], sizeof(sid));
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[0]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[0]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[1]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[1]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[2]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[2]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[3]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[3]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[4]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[4]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[5]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[5]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[6]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[6]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
        if (restSize <= 256)
        {
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[7]) * BLOCKSIZE], restSize);
            printf("%s\n", dataBuf);
            fprintf(fp, "%s",dataBuf);
            return 0;
        }
        else
        {
            restSize -= 256;
            memcpy(&dataBuf, &diskfile[(BLOCKBASE + sid.s_blocknum[7]) * BLOCKSIZE], 256);
            printf("%s", dataBuf);
            fprintf(fp, "%s",dataBuf);
        }
    }
    char* buf = (char*) &tar;
    memcpy(&diskfile[INODEBASE * BLOCKSIZE + result * INODESIZE], buf, INODESIZE);
    return 0;
}

void clearFile(int inodeNum)
{
    INODE tar;
    memcpy(&tar, &diskfile[INODEBASE * BLOCKSIZE + inodeNum * INODESIZE], INODESIZE);
    for (int i = 0; i < 4; i++)
    {
        if (tar.i_direct[i] != -1)
        {
            returnBitMap(1, tar.i_direct[i]);
            tar.i_direct[i] = -1;
            
        }
    }
    if (tar.i_sindirect != -1)
    {
        SINGLEINDIRECT sid;
        memcpy(&sid, &diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], sizeof(sid));
        for (int j = 0; j < 8; j++)
        {
            if (sid.s_blocknum[j] != -1)
            {
                returnBitMap(1, sid.s_blocknum[j]);
                sid.s_blocknum[j] = -1;
                
            }
        }
        char* buf = (char*) &sid;
        memcpy(&diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], buf, BLOCKSIZE);
        
        returnBitMap(1, tar.i_sindirect);
        tar.i_sindirect = -1;       
    }
    tar.i_size = 0;
    memcpy(&diskfile[INODEBASE * BLOCKSIZE + inodeNum * INODESIZE], &tar, INODESIZE);
}

int min(int a, int b) 
{
  return (a < b) ? a : b;
}

int writeFile(int inodeNum, int len, char* data, int beginPos)
{
    INODE tar;//remember to save
    memcpy(&tar, &diskfile[INODEBASE * BLOCKSIZE + inodeNum * INODESIZE], INODESIZE);
    timeUpdate(&tar, 0, 1, 1);
    int endPos = beginPos + len -1;
    //0, 256, 512, 768||1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816(11)
    if (beginPos < 0)
    {
        printf("Error: illegal position\n");
        return 1;
    }
    //check the end pos and arrane the space
    int endLoc = endPos/256;
    int beginLoc = beginPos/256;
    SINGLEINDIRECT sid;
    switch (endLoc)
    {
    case 0:
        if (tar.i_direct[0] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[0] = res;
        }
        break;
    case 1:
        if (tar.i_direct[0] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[0] = res;
        }
        if (tar.i_direct[1] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[1] = res;
        }
    case 2:
        if (tar.i_direct[0] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[0] = res;
        }
        if (tar.i_direct[1] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[1] = res;
        }
        if (tar.i_direct[2] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[2] = res;
        }
    case 3:
        if (tar.i_direct[0] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[0] = res;
        }
        if (tar.i_direct[1] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[1] = res;
        }
        if (tar.i_direct[2] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[2] = res;
        }
        if (tar.i_direct[3] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[3] = res;
        }
    default:
        break;
    }
    if (endLoc >=4)
    {
        if (tar.i_direct[0] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[0] = res;
        }
        if (tar.i_direct[1] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[1] = res;
        }
        if (tar.i_direct[2] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[2] = res;
        }
        if (tar.i_direct[3] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_direct[3] = res;
        }
        if (tar.i_sindirect == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            tar.i_sindirect = res;
            init_SINGLEINDIRECT(res);
        }
        
        memcpy(&sid, &diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], sizeof(sid));
        if(sid.s_blocknum[0] == -1)
        {
            int res = searchBitMap(1);
            if (res == -1)
            {
                printf("Error: not enough block\n");
                return 1;
            }
            setBitMap(1,res);
            sid.s_blocknum[0] = res;
        }
        for (int i = 5; i <= endLoc; i++)
        {
            if (sid.s_blocknum[i - 4] == -1)
            {
                int res = searchBitMap(1);
                if (res == -1)
                {
                    printf("Error: not enough block\n");
                    return 1;
                }
                setBitMap(1,res);
                sid.s_blocknum[i-4] = res;
            }
        }
        memcpy(&diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], &sid, sizeof(sid));
    }

    //update size
    int restLen = len;
    int blocknum = endLoc - beginLoc + 1;
    int* array = (int*)malloc(blocknum * sizeof(int));
    if (array == NULL)
    {
        printf("Error: fail to allocate\n");
        return 1;
    }
    int tmp = 0;
    for (int i = beginLoc; i <= endLoc; i++)
    {
        if (i < 4)
            array[tmp] = tar.i_direct[i];
        else
            array[tmp] = sid.s_blocknum[i - 4];
        tmp++;
    }
    tmp = 0;
    int currentPos = beginPos;
    int dataPos = 0;
    int firstPos = beginPos;
    while(restLen > 0)
    {
        memcpy(&diskfile[(BLOCKBASE + array[tmp]) * BLOCKSIZE + firstPos], &data[dataPos], min(256 - currentPos % 256, restLen));
        firstPos = 0;
        restLen -= 256 - currentPos % 256;
        tmp ++;
        currentPos += 256 - currentPos % 256; 
        dataPos += 256 - currentPos % 256;    
    }
    tar.i_size = beginPos + min(len, strlen(data));
    memcpy(&diskfile[INODEBASE * BLOCKSIZE + inodeNum * INODESIZE], &tar, INODESIZE);
    return 0;
    
}

int writeIn(char* name, int len, char* data)
{
    int result = searchFile(name);
    if (result == -1) 
    {
        printf("Error: no file searched.\n");
        return -1;
    }
    clearFile(result);
    int res = writeFile(result, len , data, 0); 
    return  res;
}

int insertIn(char* name, int pos, int len, char* data)
{
    int result = searchFile(name);
    //printf("sizeof: %ld\n", sizeof(data));
    if (result == -1)
    {
        printf("Error: no file searched.\n");
        return -1;
    }
    //check the size of the file/
    INODE tar;
    
    memcpy(&tar, &diskfile[INODEBASE * BLOCKSIZE + result * INODESIZE], INODESIZE);
    
    if (tar.i_size < pos)
        pos = tar.i_size;
    
    int res = writeFile(result, len, data, pos);
    return res;
}

int deleteIn(char* name, int pos, int len)
{
    int result = searchFile(name);
    //printf("sizeof: %ld\n", sizeof(data));
    if (result == -1)
    {
        printf("Error: no file searched.\n");
        return -1;
    }
    //check the size of the file/
    INODE tar;//remember to copy back
    memcpy(&tar, &diskfile[INODEBASE * BLOCKSIZE + result * INODESIZE], INODESIZE);
    if (pos >= tar.i_size)
        return 0;
    int end = pos + len - 1;
    
    int beginLoc = 0;
    int endLoc = 0;

    if (end >= tar.i_size - 1)//delete till the end of the file
    {
        end = tar.i_size - 1;
        beginLoc = pos/256;
        endLoc = end/256;
        //printf("beginLoc: %d, endLoc: %d\n", beginLoc, endLoc);
        if (beginLoc < 4)//delete sid
        {
            if (tar.i_sindirect != -1)
            {
                SINGLEINDIRECT sid;//remember to copy back
                memcpy(&sid, &diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], sizeof(sid));
                for (int i = 0; i < 8; i++)
                {
                    if (sid.s_blocknum[i] != -1)
                    {
                        returnBitMap(1, sid.s_blocknum[i]);
                        sid.s_blocknum[i]= -1;
                    }
                }
                char* buf = (char*) &sid;
                memcpy(&diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], buf, sizeof(sid));
            }
            for (int i = 3; i > 0; i--)
            {
                if(i - 1 >= beginLoc)
                {
                    if (tar.i_direct[i] != -1)
                    {
                        returnBitMap(1, tar.i_direct[i]);
                        tar.i_direct[i] = -1;
                    }
                    //printf("i is %d\n", i);
                }
                else//i is the beginLoc
                    break;
            }
            int beginPos = pos%256;//relative in a block
            memset(&diskfile[(BLOCKBASE + tar.i_direct[beginLoc]) * BLOCKSIZE + beginPos], 0, 256 - beginPos);
        }
        else//in the sid
        {
            if (tar.i_sindirect == -1)
            {
                printf("Error: delete invalid block\n");
                return 1;
            }
            SINGLEINDIRECT sid;//remember to copy back
            memcpy(&sid, &diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], sizeof(sid));
            for (int i = 7; i > 0; i--)
            {
                if(i - 1 >= beginLoc)
                {
                    if (sid.s_blocknum[i] != -1)
                    {
                        returnBitMap(1, sid.s_blocknum[i]);
                        sid.s_blocknum[i] = -1;
                    }
                }
                else//i is the beginLoc
                    break;
            }
            int beginPos = pos%256;//relative in a block
            memset(&diskfile[(BLOCKBASE + sid.s_blocknum[beginLoc - 4]) * BLOCKSIZE + beginPos], 0, 256 - beginPos);
            char* buf = (char*) &sid;
            memcpy(&diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], buf, sizeof(sid));
        }
        tar.i_size -=(end - pos + 1);
        char* buf = (char*) &tar;
        memcpy(&diskfile[INODEBASE * BLOCKSIZE + result * INODESIZE], buf, INODESIZE);
        return 0;
    }
    //first delete the content 
    char buf[3072] = {0};
    //the way I choose is to copy the rest to the buf first and use writeIn() and delete the rest 
    int locInBuf = 0;
    int copyPos = end + 1;
    int copyLoc = copyPos/256;
    int copyEndPos = tar.i_size -1;
    int copyEndLoc = copyEndPos/256;
    int tmpLoc = copyLoc;
    int tmpPos = copyPos;
    //printf("copyPos: %d, copyLoc: %d, copyendpos: %d, copyendloc %d:\n",copyPos, copyLoc, copyEndPos, copyEndLoc);
    if (copyLoc == copyEndLoc)
    {
        if (copyLoc < 4)
        {
            memcpy(&buf,&diskfile[(BLOCKBASE + tar.i_direct[copyLoc]) * BLOCKSIZE + copyPos], copyEndPos - copyPos + 1);
            locInBuf += copyEndPos - copyPos + 1;
            
        }
        else
        {
            SINGLEINDIRECT sid;
            memcpy(&sid, &diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], sizeof(sid));
            memcpy(&buf,&diskfile[(BLOCKBASE + sid.s_blocknum[copyLoc - 4]) * BLOCKSIZE + copyPos], copyEndPos - copyPos + 1);
            locInBuf += copyEndPos - copyPos + 1;
        }
    }
    else
    {
        for (tmpLoc; tmpLoc < copyEndLoc; tmpLoc++)// the last Loc manipulated out of FOR
        {
            if (tmpLoc < 4)
            {
                memcpy(&buf[locInBuf], &diskfile[(BLOCKBASE + tar.i_direct[tmpLoc]) * BLOCKSIZE + tmpPos%256], 256 - tmpPos%256);
                locInBuf += 256 - tmpPos%256;
                tmpPos = 0;
            }
            else
            {
                SINGLEINDIRECT sid;
                memcpy(&sid, &diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], sizeof(sid));
                memcpy(&buf[locInBuf], &diskfile[(BLOCKBASE + sid.s_blocknum[tmpLoc - 4]) * BLOCKSIZE + tmpPos%256], 256 - tmpPos%256);
                locInBuf += 256 - tmpPos%256;
                tmpPos = 0;
            }
        }
        //manipulate last block
        if (copyEndLoc < 4)
        {
            memcpy(&buf[locInBuf], &diskfile[(BLOCKBASE + tar.i_direct[copyEndLoc]) * BLOCKSIZE], copyEndLoc%256);
            locInBuf += copyEndLoc%256;
        }
        else
        {
            SINGLEINDIRECT sid;
            memcpy(&sid, &diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], sizeof(sid));
            memcpy(&buf[locInBuf], &diskfile[(BLOCKBASE + sid.s_blocknum[copyEndLoc]) * BLOCKSIZE], copyEndLoc%256);
            locInBuf += copyEndLoc%256;
        }   
    }
    //use writeIn()
    
    int res = writeFile(result, locInBuf, buf, pos);
    
    if (res != 0)
    {
        printf("Error: fail to write in\n");
        return 1;
    }
    //delete the rest.
    int garbagePos = pos + locInBuf;//contain this pos, the rest can be deleted;
    int garbageLoc = garbagePos/256;
    int last = tar.i_size - 1;
    beginLoc = pos/256;
    endLoc = last/256;
    
    if (garbageLoc < 4)//delete sid
    {
        if (tar.i_sindirect != -1)
        {
            SINGLEINDIRECT sid;//remember to copy back
            memcpy(&sid, &diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], sizeof(sid));
            for (int i = 0; i < 8; i++)
            {
                if (sid.s_blocknum[i] != -1)
                {
                    returnBitMap(1, sid.s_blocknum[i]);
                    sid.s_blocknum[i]= -1;
                }
            }
            char* buf = (char*) &sid;
            memcpy(&diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], buf, sizeof(sid));
        }
        for (int i = 3; i > 0; i--)
        {
            if(i - 1 >= garbageLoc)
            {
               if (tar.i_direct[i] != -1)
                {
                    returnBitMap(1, tar.i_direct[i]);
                    tar.i_direct[i] = -1;
                }
            }
            else//i is the beginLoc
                break;
        }
        int beginPos = pos%256;//relative in a block
        memset(&diskfile[(BLOCKBASE + tar.i_direct[garbageLoc]) * BLOCKSIZE + garbagePos], 0, 256 - garbagePos);
    }
    else//in the sid
    {
        if (tar.i_sindirect == -1)
        {
            printf("Error: delete invalid block\n");
            return 1;
        }
        SINGLEINDIRECT sid;//remember to copy back
        memcpy(&sid, &diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], sizeof(sid));
        for (int i = 7; i > 0; i--)
        {
               if(i - 1 >= garbageLoc)
            {
                if (sid.s_blocknum[i] != -1)
                {
                    returnBitMap(1, sid.s_blocknum[i]);
                    sid.s_blocknum[i] = -1;
                }
            }
            else//i is the beginLoc
                break;
            }
        int beginPos = pos%256;//relative in a block
        memset(&diskfile[(BLOCKBASE + sid.s_blocknum[garbageLoc - 4]) * BLOCKSIZE + garbagePos], 0, 256 - garbagePos);
        char* buf = (char*) &sid;
        memcpy(&diskfile[(BLOCKBASE + tar.i_sindirect) * BLOCKSIZE], buf, sizeof(sid));
    }
    
    
    tar.i_size -=(end - pos + 1);
    char* buffer = (char*) &tar;
    memcpy(&diskfile[INODEBASE * BLOCKSIZE + result * INODESIZE], buffer, INODESIZE);
    return 0;
}

int main(int argc, char** argv)
{
    char buffer[256] = {0};
    int fd = open("filesystem", O_RDWR | O_CREAT, 0);
    if (fd < 0)
    {
        printf("ERROR: Could not open files '%s'.\n", "filesystem");
        return -1;
    }

    long FILESIZE = BLOCKSIZE * BLOCKNUM;
    int res = lseek(fd, FILESIZE - 1, SEEK_SET);
    if (res == -1)
    {
        perror("ERROR calling lseek() to 'stretch' the file");
        close (fd);
        return -1;
    }
    res = write (fd, "", 1);
    if (res != 1) 
    {
        perror("Error writing last byte of the file");
        close(fd);
        exit(-1);
    }
    diskfile = (char* )mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (diskfile == MAP_FAILED)
    {
        close(fd);
        printf("ERROR: Could not map file.\n");
        return -1;
    }
    close(fd);
    FILE* fp = fopen ("fs.log", "w");
    char line[DATASIZE];
    int s, c;
    while (1)
    {
        printf("Please Type in commands.\n");
        if (fgets(line, sizeof(line), stdin) == NULL) 
        {
            printf("读取输入时出错\n");
            break;
        }
        /*
        char *pos;
        if ((pos = strchr(line, '\n')) != NULL) 
            *pos = '\0';  // 将回车符替换为 null 终止符
        */
        if (strcmp(line,"f\n") == 0)//f
        {
            init_FS(diskfile);
            printf("Format the file system successfully.\n");
            //printf("uid: %u.\n",currentDirectory.i_uid);
            fprintf(fp,"Done\n");
        }
        else if (line[0] == 'm' && line[1] == 'k' && line[2] == ' ')//mk
        {
            //printf ("currentDirectory ID in main: %u.\n", currentDirectory.i_id);
            
            if (sb.s_free_inodes_count <= 0 || sb.s_free_blocks_count <= 0)
            {
                printf("Error: not enough space.\n");
                fprintf(fp,"No\n");
                continue;
            }
            char fileName[MAX_COMMANDS] = {0};
            sscanf(line, "mk %[^\n]", fileName);
            //printf("FILENAME: %s\n", fileName);
            int res = createFile(fileName);
            if (res == -1)
            {
                printf("Failed to create the file %s.\n", fileName);
                fprintf(fp,"No\n");
                continue;
            }
            int dirNum = currentDirectory.i_direct[0];
            FILEDIRECTORY cd;
            memcpy(&cd, &diskfile[(BLOCKBASE + dirNum) * BLOCKSIZE ], sizeof(cd));
            printf("The directroy where you create the file is: %s\n", cd.f_fileName);
            //printf("currentDirectory.i_direct[1] : %u.\n", currentDirectory.i_direct[1]);
            fprintf(fp,"Yes\n");
           
        }
        else if (line[0] == 'm' && line[1] == 'k' && line[2] == 'd' && line[3] == 'i' && line[4] == 'r' && line[5] == ' ')//mkdir
        {
            if (sb.s_free_inodes_count <= 0 || sb.s_free_blocks_count <= 0)
            {
                printf("Error: not enough space.\n");
                fprintf(fp,"No\n");
                continue;
            }

            char directoryName[MAX_COMMANDS] = {0};
            sscanf(line, "mkdir %[^\n]", directoryName);
            
            int res = createDirectory(directoryName);
            printf ("currentDirectory ID in main: %u.\n", currentDirectory.i_id);
            fprintf(fp,"Yes\n");
        }
        else if (line[0] == 'r' && line[1] == 'm' && line[2] == ' ')
        {
            char deleteName[MAX_COMMANDS] = {0};
            sscanf(line, "rm %[^\n]", deleteName);
            
            FILEDIRECTORY fd;
            memcpy(&fd, &diskfile[(BLOCKBASE + currentDirectory.i_direct[0]) * BLOCKSIZE + 32], sizeof(fd));
            //printf("now the name is %s.\n",fd.f_fileName);

            int ans = deleteFile(deleteName);
            //printf("ans is: %d.\n", ans);
            //printf("now nums: %u \n", currentDirectory.i_uid);
            memcpy(&fd, &diskfile[(BLOCKBASE + currentDirectory.i_direct[0]) * BLOCKSIZE + 32], sizeof(fd));
            //printf("now the name is %s.\n",fd.f_fileName);
            fprintf(fp,"Yes\n");
        }
        else if(line[0] == 'c' && line[1] =='d' && line[2] == ' ')
        {
            //printf("current directory at the beginning : %d\n",currentDirectoryInode);
            char pathName[MAX_COMMANDS] ={0};
            sscanf(line, "cd %[^\n]", pathName);
            int oriInode = currentDirectory.i_id;

            delchar(pathName, strlen(pathName) + 1);
            char* directoryName = strtok(pathName, "/"); 
            int tmpInode = -1; //represent current directory inode
            if (pathName[0] =='/')
            {
                char* buf = (char*) &currentDirectory;
                tmpInode = 0;
                memcpy(&diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE], buf, INODESIZE);
                currentDirectoryInode = 0;
                memcpy(&currentDirectory, &diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE], INODESIZE);    
            }
           
            
            while (directoryName != NULL)
            {
                if (strcmp(directoryName, "..") == 0)
                {
                    tmpInode = currentDirectory.i_gid; //get the father.
                    if (tmpInode == -1)
                    {
                        printf("Error: no directory.\n");
                        fprintf(fp,"No\n");
                        currentDirectoryInode = oriInode;
                        memcpy(&currentDirectory, &diskfile[(INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE)], INODESIZE); 
                        break;
                    }
                    char *buf = (char*)&currentDirectory;
                    memcpy(&diskfile[(INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE)], buf, INODESIZE);
                    currentDirectoryInode = tmpInode;
                    memcpy(&currentDirectory, &diskfile[(INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE)], INODESIZE);
                }
                else if (strcmp(directoryName, ".") == 0)
                {
                    tmpInode = 0; //get the father
                    char *buf = (char*)&currentDirectory;
                    memcpy(&diskfile[(INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE)], buf, INODESIZE);
                    currentDirectoryInode = tmpInode;
                    memcpy(&currentDirectory, &diskfile[(INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE)], INODESIZE); 
                }
                else
                {
                    int result = searchInDir(directoryName);
                    if (result == -1)
                    {
                        printf("Error: no directory searched.\n");
                        currentDirectoryInode = oriInode;
                        memcpy(&currentDirectory, &diskfile[(INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE)], INODESIZE); 
                        fprintf(fp,"No\n");
                        break;
                    }
                    char *buf = (char*)&currentDirectory;
                    memcpy(&diskfile[(INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE)], buf, INODESIZE);
                    currentDirectoryInode = result;
                    memcpy(&currentDirectory, &diskfile[(INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE)], INODESIZE); 
                }
                directoryName = strtok(NULL, "/");
            }
            INODE cd;
            memcpy(&cd, &diskfile[INODEBASE * BLOCKSIZE + currentDirectoryInode * INODESIZE], INODESIZE);
            FILEDIRECTORY cdName;
            memcpy(&cdName, &diskfile[(BLOCKBASE + cd.i_direct[0]) * BLOCKSIZE], sizeof(cdName));
            printf("Current directory:%s\n",cdName.f_fileName);
            fprintf(fp,"Yes\n");            
        }
        else if (line[0] == 'r' && line[1] == 'm' && line[2] == 'd' && line[3] == 'i' && line[4] == 'r' && line[5] == ' ')
        {
            char deletePath[MAX_COMMANDS] ={0};
            sscanf(line, "rmdir %[^\n]", deletePath);
            int res = deleteDir(deletePath);
            if (res == -1)
            {
                printf("Error: fail to delete.\n");
                fprintf(fp,"No\n");
            }
            else
                fprintf(fp,"Yes\n");
        }
        else if (strcmp(line, "ls\n") == 0)
        {
            int res = listing(fp);
            if (res != 0)
            {
                printf("Error: failed to list.\n");
                fprintf(fp,"No\n");
            }
        }
        else if (line[0] == 'c' && line[1] == 'a' && line[2] == 't' && line[3] == ' ')
        {
            char file[MAX_COMMANDS] = {0};
            sscanf(line, "cat %[^\n]", file);
            res =  catFile(file, fp);
            if (res != 0)
            {
                printf("Error: fail to cat %s\n", file);
                fprintf(fp,"No\n");
            }
            else
                fprintf(fp, "\n");
        }
        else if (line[0] == 'w' && line[1] == ' ')
        {
            char name[MAX_COMMANDS] = {0};
            _u32 len = 0;
            char buf[DATASIZE] = {0};
            sscanf(line, "w %s %u %[^\n]", name, &len, buf);
            printf("name: %s\nlen: %u\nData: %s\n", name, len, buf); 
            
            res = writeIn(name, len, buf);
            if (res != 0)
            {
                printf("Error: fail to write\n");
                fprintf(fp,"No\n");
            }
            else
            {
                char tmp[256] = {0};;
                int node = searchFile(name);
                INODE tar;//remember to save
                memcpy(&tar, &diskfile[INODEBASE * BLOCKSIZE + node * INODESIZE], INODESIZE);
                memcpy(&tmp, &diskfile[(BLOCKBASE + tar.i_direct[0])* BLOCKSIZE], len);
                printf("%s\n", tmp);
                fprintf(fp,"Yes\n");
            }
        }
        else if (line[0] == 'i' && line[1] == ' ')
        {
            char name[MAX_COMMANDS] = {0};
            _u32 len = 0;
            _u32 pos = 0;
            char buf[DATASIZE] = {0};
            sscanf(line, "i %s %u %u %[^\n]", name, &pos, &len, buf);
            int res = insertIn(name, pos, len, buf);
            if (res != 0)
            {
                printf("Error: fail to insert\n");
                fprintf(fp,"No\n");
            }
            else
                fprintf(fp,"Yes\n");

        }
        else if(line[0] == 'd' && line[1] == ' ')
        {
            char name[MAX_COMMANDS] = {0};
            _u32 len = 0;
            _u32 pos = 0;
            sscanf(line, "d %s %u %u", name, &pos, &len);
            int res = deleteIn(name, pos, len);
            if (res != 0 )
            {
                printf("Error: fail to delete\n");
                fprintf(fp,"No\n");
            }
            else
                fprintf(fp,"Yes\n");
        }
        else if (strcmp(line, "e\n") == 0)
        {
            
            int result = munmap(diskfile, FILESIZE);
            if (result == -1)
            {
                printf("Error: fail to mumap\n");
            }
            else
            {
                printf("Goodbye\n");
                fprintf(fp,"Goodbye!\n");
                return 0;
            }
        }
        else
        {
            printf("Invalid command.\n");
        }
    }


}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       