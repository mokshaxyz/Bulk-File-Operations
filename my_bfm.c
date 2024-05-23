#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>

#define USER_ERROR       -3
#define E_OK              0
#define BUF_SIZE          1054
#define MAX_TEXT_LENGTH   50
#define LOG_FILE          "log.txt" // Assuming the log file name will always be the same
#define MAX_PATH_LEN      1054

char* PARENT_DIRECTORY = NULL;
int status = E_OK;

//Functions:
// void how_to();
int create_file_or_directory(int switch_num, const char *path);
int rename_file_or_directory(const char *old_path, const char *new_name);
int delete_file_or_directory(const char *path);
int append_text(const char *path, const char *text);
int append_binary(const char* path, int start_number);
int create_log_file(const char *log_file_name);
// int get_parent_directory(const char *file_or_directory);
int log_action(char *action, const char* path);
char* get_parent_path(const char* path);


struct linux_dirent64 {
    ino64_t        d_ino;    /* 64-bit inode number */
    off64_t        d_off;    /* 64-bit offset to next structure */
    unsigned short d_reclen; /* Size of this dirent */
    unsigned char  d_type;   /* File type */
    char           d_name[]; /* Filename (null-terminated) */
};

// Optionally Create the file or the directory. (Command line switch: -c <new dir name>)
// Add at the end of the file atmost 50 bytes of text if it is a text file, or 50 bytes of even numbers between [51,199] in a sequence for a binary file. This is ignored if a directory is given (Discussion point: Should an "error" be returned?).
// For text file, the text to add is given as a string on the command line. (Command line switch: -s "The 50 Byte String")
// For a binary file the starting even number is given on the command line. (Command line switch: -e <starting even number>)
// Optionally Delete the file or an empty directory. Non-empty directories are deleted after first deleting all the files contained within it. (Command line switch: -d <dir name>)
// Optionally Rename the file or directory. (Command line switch: -r <file or dir old name> <new name>)
// Generate a log file that records the actions performed in each object (file or dir) found within the given directory. If the log file already exists then append the new log. If the log file does not exist, then create one and write to it. (Command line switch: -l <log file name>)


int main(int argc, char* argv[])
{
    // if (argc < 2) 
    // {   how_to();
    //     return USER_ERROR;
    // }

    if (argv[1][0] == '-') {
        switch(argv[1][1]){
            case 'c':
                if(strcmp(argv[2],"f")==0){
                    status = create_file_or_directory(2,argv[3]);
                }else if(strcmp(argv[2],"d")==0){
                    status = create_file_or_directory(1,argv[3]);
                }
                break;
            case 's':
                status = append_text(argv[2],argv[3]);
                break;
            case 'e':
                int temp = atoi(argv[3]);
                status = append_binary(argv[2], temp);
                break;
            case 'd':
                status = delete_file_or_directory(argv[2]);
                break;
            case 'r':
                status = rename_file_or_directory(argv[2], argv[3]);
                break;
            case 'l':
                status = create_log_file(argv[2]);
                break;
            default:
                // how_to();
                return USER_ERROR; 
        }
    }else{
        // how_to();
        return USER_ERROR;
    }
    return status;
}

// void how_to()
// {
//     char mesg[] = "please print in this way";

//     if(write(1,mesg,sizeof(mesg))==-1)
//     {
//         return errno;
//     }  
// }

int create_file_or_directory(int switch_num,const char *path)
{
    char* action = NULL;
    if(switch_num == 1){
        if(mkdir(path, 0755) == 0){
            action = "\nCreated the directory:";
            log_action(action, path);
            return E_OK;
        }
        if(errno == -1){
            return errno;
        }
    }
    if(switch_num==2){
        {
            int fd = open(path, O_CREAT | O_EXCL | O_WRONLY, 0755);
            if (fd != -1){
                close(fd);
                action ="\nCreated the file: ";
                log_action(action,path);
                return E_OK;
            } else {
                return errno;
            }
        }
    }else{
        //help()
        return USER_ERROR;
    }
    action = "\nDeleted";
    log_action(action,path);
    return E_OK;
}

int delete_file_or_directory(const char *path) {
    struct stat file_stat;
    if (stat(path, &file_stat) == -1) {
        return errno;
    }

    if (S_ISDIR(file_stat.st_mode)) {
        // If the path is a directory
        int dir_fd = open(path, O_RDONLY);
        if (dir_fd == -1) {
            return errno;
        }

        char buf[BUF_SIZE];
        struct linux_dirent64 *entry;
        ssize_t number_read;

        while ((number_read = getdents64(dir_fd, buf, BUF_SIZE)) > 0) {
            for (int byte_position = 0; byte_position < number_read;) {
                entry = (struct linux_dirent64*)(buf + byte_position);

                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    // If entry is not "." or ".."
                    char full_path[BUF_SIZE];
                    snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

                    if (delete_file_or_directory(full_path) == -1) {
                        close(dir_fd);
                        return errno;
                    }
                }
                byte_position += entry->d_reclen;
            }
        }

        close(dir_fd);
        
        if (rmdir(path) == -1) {
            return errno;
        }else{
            char* action = "\n Deleted :";
            log_action(action, path);
            return E_OK;
        }
    } else {
        // If the path is a file
        if (unlink(path) == -1) {
            return errno;
        }else{
            char* action = "\n Deleted :";
            log_action(action, path);
            return E_OK;
        }
    }
    char* action = "\n Deleted :";
    log_action(action, path);

    return E_OK; // Return 0 for success
}

int rename_file_or_directory(const char *old_path, const char *new_name) {
    struct stat stat_buf;
    if (stat(old_path, &stat_buf) != 0) {
        return errno;
    }

    if (S_ISDIR(stat_buf.st_mode)) {
        if (rename(old_path, new_name) == -1) {
            return errno;
        }
    } else {
        if (rename(old_path, new_name) == -1) {
            return errno;
        }
    }  
    char* action = "\nRenamed to: ";
    log_action(action, new_name);
    return E_OK;
}

int append_text(const char *path, const char *text){
    int save_errno = 0;
    int fd = open(path, O_WRONLY | O_APPEND);
    //if a directory, errno will be set appropriately. 

    if (fd == -1){
        return errno;
    }

    ssize_t text_len = strlen(text);
    ssize_t bytes_written;
    
    if(text_len > MAX_TEXT_LENGTH){
        bytes_written = write(fd, text, MAX_TEXT_LENGTH);

    }else{
        bytes_written = write(fd, text, text_len);
    }

    if (bytes_written == -1){
        save_errno = errno;
        close(fd);
        return save_errno;
    }
    //log_action("String added to the end\n",path);
    close(fd);
    char* action = "\nText appended in: ";
    log_action(action,path);
    return E_OK;
}

int append_binary(const char* path, int start_number){
    int save_errno = 0;
    int fd = open(path, O_WRONLY | O_APPEND);

    if (fd == -1){
        return errno;
    }

    if (start_number > 51 && start_number < 199){
        
        if(start_number%2 != 0){
            start_number++;
        }
        for (short int i = start_number; i < 199; i += 2) {
            ssize_t bytes_written = write(fd, &i, sizeof(short int));
            if (bytes_written == -1){
                save_errno = errno;
                close(fd);
                return save_errno;
            }
        }
    }
    //log_action("Binary added to the end\n",path);
    close(fd);
    char* action = "\nBinary sequence appended in: ";
    log_action(action,path);
    return E_OK;
}

int log_action(char *action, const char* path){
    PARENT_DIRECTORY = get_parent_path(path);
    int log_fd = open(PARENT_DIRECTORY, O_WRONLY | O_APPEND| O_CREAT, 0666); // Specify file mode 0666 for read and write permissions

    if (log_fd == -1) {
        return errno;
    }

    ssize_t bytes_written = write(log_fd, action, strlen(action)); // Write the action to the log file
    if (bytes_written == -1) {
        close(log_fd);
        return errno;
    }

    bytes_written = write(log_fd, path, strlen(path));
    if (bytes_written == -1) {
        close(log_fd);
        return errno;
    }

    close(log_fd);
    return E_OK; // Return 0 for success
}


int create_log_file(const char *log_file_name){
    int fd = open(log_file_name, O_WRONLY | O_APPEND | O_CREAT, 0666); // Specify file mode 0666 for read and write permissions

    if (fd == -1){
        return errno;
    }

    close(fd);
    return E_OK; // Return 0 for success
}

char* get_parent_path(const char* path) {
    char* parent_path = strdup(path); // Create a copy of the path
    if (parent_path == NULL) {
        // Memory allocation failed
        exit(EXIT_FAILURE);
    }

    // Find the last occurrence of '/' in the path
    char* last_slash = strrchr(parent_path, '/');
    if (last_slash == NULL) {
        // No parent path found (it's the root directory)
        free(parent_path);
        return NULL;
    }

    // Replace the last slash with '\0' to remove the trailing slash
    *last_slash = '\0';

    // Append "/log.txt" to the parent path
    parent_path = realloc(parent_path, strlen(parent_path) + strlen("/log.txt") + 1);
    if (parent_path == NULL) {
        // Memory allocation failed
        exit(EXIT_FAILURE);
    }
    strcat(parent_path, "/log.txt");

    return parent_path;
}
