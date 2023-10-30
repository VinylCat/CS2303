```
Prj2+521021911051
	step1
		disk.c
		disk.log
		disk_storage_file
	step2
		fs.c
		fs.log
		filesystem
	step3
		client.c
		disk.c
		disk.log
		disk_storage_file
		fs.c
		fs.log
		filesystem
	report.pdf
	report.md
	Typescript.pdf
	Prj3README
```

In step 2 and 3 , `ls` will list the file in the order of create time.

in step 3, you should launch programs in the sequence of `disk`, `fs`, `client`.

Also, if you want to test shut down, you should also close the programs in the sequence of `client`,`fs`.

If you shut down `fs`, you should type in `r` command first in the `client` to recover the file system.