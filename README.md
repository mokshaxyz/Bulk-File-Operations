
### Collaborators: Moksha Shah and Kalash Shah

## Overview:
This C program manipulates a given file or directory using system calls. The user can provide specifiers depending on what they want to achieve. 

Flags: 

1. i. -c f creates a file depending on the path provided by the user
  ii. -c d created a directory depending on the path provided by the user

2. -s writes the text given by the user as a string to the end of the file at the given path

3. -d deletes the file or an empty directory found at the provided path

4. -r renames a given file

5. -e writes 50 bytes of even numbers between 51 and 199 according to the starting point specified by the user

6. -l generates a log file and records the actions performed in file or directory found within the given directory


### How to use the code:

#### Compiling and Execution:

```Bash
        make
``` 

For information on how to create a directory or append to binary file, please read below.

###### Append
```Bash
        ./my_fm -s <Path>  "string to append" # For either a binary or a text file
```
####### Binary:
```Bash
        ./my_fm -e <Path>  <Integer> # For either a binary or a text file
```
###### Rename
```Bash
        ./my_fm -r <OldPath> <NewPath>  # For either a file or a directory 
```
###### Create (directory or file)
```Bash
        ./my_fm -c f <NewFileName> # For a file
        ./my_fm -c d <NewDirectoryName> # For a Directory
```

###### Delete
```Bash
        ./my_fm  -d <path> # Will delete a file or a directory, can take a path as an input or relative path.  
```


### Behvaioural Choices and some explainations:

The delete funciton makes a recursive call, and keeps on deleting all the file and the directory present in the path given. 

The log file will always be created log.txt, in the parent directory.
the user should always giev the log file name to be log.txt

To get the parent directory, string manipulation has been done, which might not be the best approach, however, was seem best considering the assignment.

The flags are explained above, and they just are the choice of the programmers.
