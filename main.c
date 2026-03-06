#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct {
    char name[255];
    int size;
} File;

typedef struct {
    File *file;
    int count;
} Files;

void file_append_cstr(Files *files, char *cstr) {
    if (files->count == 0) {
        files->file = malloc(sizeof(File));
    } else {
        files->file = realloc(files->file, (files->count * sizeof(File)) + sizeof(File));
    }

    assert(files->file != NULL);

    strcpy(files->file[files->count++].name, cstr);
}

int find_files(char *path, Files *files) {
    struct dirent *dir;
    DIR *dirptr = opendir(path);
    
    while ((dir = readdir(dirptr)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            file_append_cstr(files, dir->d_name);
        }
    }

    closedir(dirptr);
    return 0;
}


// TODO: some kind of preprocessor possibly
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "error: not enough arguments\n");
        fprintf(stderr, "usage: %s <directory>\n", argv[0]);
        return 1;
    }

    char *path = argv[1];

    Files files = {0};
    find_files(path, &files);

    for (int i = 0; i < files.count; ++i) {
        assert(sizeof(path) + sizeof(files.file[i].name) >= 255);

        char full_path[255];
        char name_pdf_ext[255];

        strcat(full_path, path);
        if (path[strlen(path)] != '/') strcat(full_path, "/");
        strcat(full_path, files.file[i].name);

        strncpy(name_pdf_ext, files.file[i].name, strlen(files.file[i].name) - 3);
        strcat(name_pdf_ext, ".pdf");

        // pandoc -o $input_name.pdf --from markdown --to pdf $input_name.md --toc
        char *pandoc_args[] = {
            "",
            "-o",
            name_pdf_ext,
            "--from",
            "markdown",
            "--to",
            "pdf",
            full_path,
            "--toc",
            NULL,
        };

        printf("debug: arguments = [");
        for (int i = 1; pandoc_args[i] != NULL; ++i) {
            printf("%s", pandoc_args[i]);
            if (pandoc_args[i + 1] == NULL) printf("]\n");
            else printf(", ");
        }
        
        printf("log: processing %s to %s\n", full_path, name_pdf_ext);
        
        pid_t pid;
        pid = fork(); // begin child process

        int stats;

        if (pid == 0) {
            if (execvp("/usr/bin/pandoc", pandoc_args) == -1) {
                fprintf(stderr, "error: could not execute pandoc\n");
                return 1;
            }
            exit(0); // exit from child process
        } else {
            waitpid(pid, &stats, 0);

            if (stats == 0)
                printf("debug: child process ended nicely: pid %d\n", stats);
            else if (stats == 1)
                printf("debug: child process ended with error: pid %d\n", stats);
        }
        
        // reset strings
        *full_path = '\0';
        *name_pdf_ext = '\0';
    }
    return 0;
}
