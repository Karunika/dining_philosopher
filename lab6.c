#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

#define BUFFER_SIZE 4096

void copy_read_write(int fd_from, int fd_to) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    while ((bytes_read = read(fd_from, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(fd_to, buffer, bytes_read);
        if (bytes_written < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
}

void copy_mmap(int fd_from, int fd_to) {
    struct stat file_stat;
    fstat(fd_from, &file_stat);

    void* file_in_memory = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd_from, 0);
    if (file_in_memory == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if (write(fd_to, file_in_memory, file_stat.st_size) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    if (munmap(file_in_memory, file_stat.st_size) < 0) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

void print_help() {
    printf("Usage: copy [-m] <file_name> <new_file_name>\n");
    printf("       copy [-h]\n");
    printf("\n");
    printf("Options:\n");
    printf("  -m      Use mmap() and memcpy() for file copying\n");
    printf("  -h      Display this help message\n");
}

int main(int argc, char *argv[]) {
    int use_mmap = 0;
    int option;
    
    while ((option = getopt(argc, argv, "mh")) != -1) {
        switch (option) {
            case 'm':
                use_mmap = 1;
                break;
            case 'h':
                print_help();
                return EXIT_SUCCESS;
            default:
                print_help();
                return EXIT_FAILURE;
        }
    }

    if (argc - optind != 2) {
        print_help();
        return EXIT_FAILURE;
    }

    char *file_name = argv[optind];
    char *new_file_name = argv[optind + 1];

    int fd_from = open(file_name, O_RDONLY);
    if (fd_from < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    int fd_to = open(new_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd_to < 0) {
        perror("open");
        close(fd_from);
        return EXIT_FAILURE;
    }

    if (use_mmap) {
        copy_mmap(fd_from, fd_to);
    } else {
        copy_read_write(fd_from, fd_to);
    }

    close(fd_from);
    close(fd_to);

    return EXIT_SUCCESS;
}
