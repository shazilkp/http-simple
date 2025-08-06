#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>


void print_dirent(const struct dirent* dp) {
    printf("Name     : %s\n", dp->d_name);
    printf("Inode    : %lu\n", (unsigned long)dp->d_ino);
    printf("Offset   : %ld\n", (long)dp->d_off);
    printf("RecordLen: %hu\n", dp->d_reclen);

    // Decode file type
    const char *type_str;
    switch (dp->d_type) {
        case DT_REG:  type_str = "Regular file"; break;
        case DT_DIR:  type_str = "Directory"; break;
        case DT_LNK:  type_str = "Symbolic link"; break;
        case DT_FIFO: type_str = "FIFO/pipe"; break;
        case DT_SOCK: type_str = "Socket"; break;
        case DT_CHR:  type_str = "Character device"; break;
        case DT_BLK:  type_str = "Block device"; break;
        case DT_UNKNOWN:
        default:      type_str = "Unknown"; break;
    }

    printf("Type     : %s\n", type_str);
    printf("--------------------------\n");
}

static void lookup()
{
    DIR *dirp;
    struct dirent *dp;


    if ((dirp = opendir(".")) == NULL) {
        perror("couldn't open '.'");
        return;
    }

    printf(
    "<html><head><title>Index of Files</title></head><body><h1>Index of Files</h1><table>");
    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
            
            printf("<tr><td><a href=\"%s\">%s</a></td></tr>",
                dp->d_name, dp->d_name);
                
            }
        } while (dp != NULL);
        
    printf("</table></body></html>");

    if (errno != 0){
        perror("error reading directory");
    }
        
    (void) closedir(dirp);
    return;
}


int main(int argc, char *argv[])
{
    int i;
    lookup();
    return (0);
}