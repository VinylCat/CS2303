# Typescript

Here I will display the valid input of each program.

## Step 1

```c
./disk 60 3 1000 disk_storage_file
请输入指令：
I
请输入指令：
R 4 60
请输入指令：
W 0 0 hello
请输入指令：
R 3 0
请输入指令：
www
Commands error. Please retype in the cpmmands.
请输入指令：
R 0 0
请输入指令：
E
退出程序
```

```
//disk.log
60 3
No
Yes
Yes
Yes hello
end
```

## Step 2

```c
./fs
Please Type in commands.
mk a
Error: not enough space.//didn't format, so no space
Please Type in commands.
f
Format the file system successfully.
Please Type in commands.
mk a   
The directroy where you create the file is: root
Please Type in commands.
mkdir b
currentDirectory ID in main: 0.
Please Type in commands.
ls
Current directory:
root
Files:
a filesize: 0
Access Time：2023-06-03 22:13:10
Create Time：2023-06-03 22:13:10
Modify Time：2023-06-03 22:13:10
SonDirectory:
b
Please Type in commands.
cd b
Current directory:b
Please Type in commands.
cd c
Error: no directory searched.//didn't exist c
Current directory:b
Please Type in commands.
cd .
Current directory:root
Please Type in commands.
w a 10 hello world
name: a
len: 10
Data: hello world
hello worl
Please Type in commands.
i a 2 2 asd
Please Type in commands.
cat a
Reading a ...
The size is 12
Data is printed below:
heasllo worl
Please Type in commands.
i a 14 3 end
Please Type in commands.
cat a
Reading a ...
The size is 15
Data is printed below:
heasllo worlend
Please Type in commands.
d a 2 2
Please Type in commands.
cat a
Reading a ...
The size is 13
Data is printed below:
hello worlend
Please Type in commands.
d a 13 18
Please Type in commands.
cat a
Reading a ...
The size is 13
Data is printed below:
hello worlend
Please Type in commands.
d a 10 18
Please Type in commands.
cat a
Reading a ...
The size is 10
Data is printed below:
hello worl
Please Type in commands.
rm a
Please Type in commands.
ls
Current directory:
root
Files:
SonDirectory:
b
Please Type in commands.
rmdir b
Please Type in commands.
ls
Current directory:
root
Files:
SonDirectory:
Please Type in commands.
e
Goodbye
```

```c
//fs.log
No
Done
Yes
Yes
Yes
Yes apple banana &work 
Yes
Yes 
Yes
Yes apple banana &work 
Yes
Yes apple banana &work 
Yes Hello World
Yes
Yes Hello,
Goodbye!
```

Because `i` command in fact use `w` command twice, so in log file it looks wired.

## Step 3

You should launch programs in the sequence of `disk`, `fs`, `client`.

Also, if you want to test shut down, you should also close the programs in the sequence of `client`,`fs`.

If you shut down `fs`, you should type in `r` command first in the `client` to recover the file system.

```
client:
Please type in the command
mk a
No
Please type in the command
f
Done
Please type in the command
mk a
Yes
Please type in the command
ls
Yes
a filesize 0 2023-06-03 22:29:52 2023-06-03 22:29:52 2023-06-03 22:29:52

Please type in the command
w a 9 helloooasd
Yes
Please type in the command
cat a
Yes
helloooas
Please type in the command
ls  
Yes
a filesize 9 2023-06-03 22:30:05 2023-06-03 22:29:52 2023-06-03 22:30:05

Please type in the command
i a 4 4 halio
Yes
Please type in the command
cat a
Yes
hellhalioooas
Please type in the command
rm a
Yes
Please type in the command
ls
Yes

Please type in the command
E
```

The output of `fs` is almost like step 2

```
disk:
Connected successfully
c: 0 s: 0 DELAY: 0
c: 0 s: 0 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 2 DELAY: 0
c: 0 s: 2 DELAY: 0
c: 11 s: 1 DELAY: 11000
c: 11 s: 1 DELAY: 0
c: 0 s: 2 DELAY: 11000
c: 0 s: 2 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 2 DELAY: 0
c: 11 s: 1 DELAY: 11000
c: 11 s: 1 DELAY: 0
c: 0 s: 2 DELAY: 11000
c: 11 s: 1 DELAY: 11000
c: 0 s: 2 DELAY: 11000
c: 11 s: 1 DELAY: 11000
c: 11 s: 1 DELAY: 0
c: 0 s: 2 DELAY: 11000
c: 0 s: 2 DELAY: 0
c: 11 s: 1 DELAY: 11000
c: 0 s: 2 DELAY: 11000
c: 0 s: 1 DELAY: 0
c: 11 s: 2 DELAY: 11000
c: 0 s: 1 DELAY: 11000
c: 0 s: 2 DELAY: 0
c: 0 s: 2 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 12 s: 0 DELAY: 12000
c: 0 s: 2 DELAY: 12000
c: 11 s: 1 DELAY: 11000
c: 0 s: 2 DELAY: 11000
c: 12 s: 0 DELAY: 12000
c: 0 s: 2 DELAY: 12000
c: 11 s: 1 DELAY: 11000
c: 0 s: 2 DELAY: 11000
c: 12 s: 0 DELAY: 12000
c: 0 s: 2 DELAY: 12000
c: 11 s: 1 DELAY: 11000
c: 11 s: 1 DELAY: 0
c: 0 s: 2 DELAY: 11000
c: 0 s: 2 DELAY: 0
c: 11 s: 1 DELAY: 11000
c: 0 s: 2 DELAY: 11000
c: 12 s: 0 DELAY: 12000
c: 0 s: 2 DELAY: 12000
c: 12 s: 0 DELAY: 12000
c: 0 s: 2 DELAY: 12000
c: 0 s: 2 DELAY: 0
c: 12 s: 0 DELAY: 12000
c: 0 s: 2 DELAY: 12000
c: 0 s: 2 DELAY: 0
c: 11 s: 1 DELAY: 11000
c: 0 s: 2 DELAY: 11000
c: 12 s: 0 DELAY: 12000
c: 0 s: 2 DELAY: 12000
c: 11 s: 1 DELAY: 11000
c: 11 s: 1 DELAY: 0
c: 11 s: 1 DELAY: 0
c: 0 s: 2 DELAY: 11000
c: 0 s: 1 DELAY: 0
c: 12 s: 0 DELAY: 12000
c: 0 s: 1 DELAY: 12000
c: 0 s: 1 DELAY: 0
c: 0 s: 2 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 11 s: 1 DELAY: 11000
c: 0 s: 2 DELAY: 11000
c: 11 s: 1 DELAY: 11000
c: 0 s: 2 DELAY: 11000
c: 0 s: 0 DELAY: 0
退出程序
```

If you shut down the client and reconnect, `fs`will be like:

```
fs:
Oops!Connection failed.
New connection is built.
```

If you shut down `client ` and `fs` and relaunch them:

```
disk:
Oops!Connection failed.
New connection is built.
c: 0 s: 0 DELAY: 0
c: 0 s: 0 DELAY: 0
c: 0 s: 1 DELAY: 0
c: 0 s: 2 DELAY: 0
c: 11 s: 1 DELAY: 11000
c: 11 s: 1 DELAY: 0
c: 0 s: 2 DELAY: 11000
c: 12 s: 1 DELAY: 12000
c: 0 s: 2 DELAY: 12000
c: 12 s: 0 DELAY: 12000
c: 0 s: 2 DELAY: 12000
```

 ```
 fs:
 Please Type in commands.
 Here is the message: r
 
 currentDirectory.i_id:0, currentDirectory.i_uid:2
 Filesystem is recovered to Root.
 Please Type in commands.
 Here is the message: ls
 
 Current directory:
 root
 Files:
 a filesize: 0
 Access Time：2023-06-03 22:34:13
 Create Time：2023-06-03 22:34:13
 Modify Time：2023-06-03 22:34:13
 SonDirectory:
 asd
 Please Type in commands.
 ```

```
client:
Please type in the command
r
Recover
Please type in the command
ls
Yes
a filesize 0 2023-06-03 22:34:13 2023-06-03 22:34:13 2023-06-03 22:34:13
&asd 
```



