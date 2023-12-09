#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

typedef struct
{
    unsigned int width;
    unsigned int height;
} BMPHeader;

int output_descriptor;

void usage(int argc, char *argv[])
{
    if (argc != 2)
    {
        char error[100];
        sprintf(error, "Usage: %s <director_intrare>\n", argv[0]);
        printf("%s", error);
        exit(EXIT_FAILURE);
    }
}

int open_file(char *name)
{
    int descriptor;
    if ((descriptor = open(name, O_RDONLY)) < 0)
    {
        perror("Error opening input file\n");
        exit(EXIT_FAILURE);
    }
    return descriptor;
}

BMPHeader read_header(int descriptor, BMPHeader header)
{
    if (lseek(descriptor, 18, SEEK_SET) < 0)
    {
        perror("Error moving cursor\n");
        exit(EXIT_FAILURE);
    }

    if (read(descriptor, &header, sizeof(header)) != sizeof(header))
    {
        perror("Error reading image header\n");
        close(descriptor);
        exit(EXIT_FAILURE);
    }
    return header;
}

int open_txt()
{
    int descriptor;
    if ((descriptor = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0)
    {
        perror("Error creating output file\n");
        exit(EXIT_FAILURE);
    }
    return descriptor;
}

char *get_permissions(char type, mode_t mode)
{
    char *temp = (char *)malloc(4);
    if (temp == NULL)
    {
        perror("Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    switch (type)
    {
    case 'u':
        temp[0] = (mode & S_IRUSR) ? 'R' : '-';
        temp[1] = (mode & S_IWUSR) ? 'W' : '-';
        temp[2] = (mode & S_IXUSR) ? 'X' : '-';
        break;
    case 'g':
        temp[0] = (mode & S_IRGRP) ? 'R' : '-';
        temp[1] = (mode & S_IWGRP) ? 'W' : '-';
        temp[2] = (mode & S_IXGRP) ? 'X' : '-';
        break;
    case 'o':
        temp[0] = (mode & S_IROTH) ? 'R' : '-';
        temp[1] = (mode & S_IWOTH) ? 'W' : '-';
        temp[2] = (mode & S_IXOTH) ? 'X' : '-';
        break;
    }
    temp[3] = '\0';

    return temp;
}

void print_stats_bmp(int image_descr, int out_descr, BMPHeader header, char *name)
{
    char output[1000];
    struct stat image_info;

    if (fstat(image_descr, &image_info) < 0)
    {
        perror("Error reading file information\n");
        exit(EXIT_FAILURE);
    }

    char date[12];
    strftime(date, sizeof(date), "%d.%m.%Y", gmtime(&image_info.st_mtime));

    char *user = get_permissions('u', image_info.st_mode);
    char *group = get_permissions('g', image_info.st_mode);
    char *other = get_permissions('o', image_info.st_mode);

    int output_size = sprintf(output,
                              "nume fisier: %s\n"
                              "inaltime: %d\n"
                              "lungime: %d\n"
                              "dimensiune: %ld\n"
                              "identificatorul utilizatorului: %d\n"
                              "timpul ultimei modificari: %s\n"
                              "contorul de legaturi: %ld\n"
                              "drepturi de acces user: %s\n"
                              "drepturi de acces grup: %s\n"
                              "drepturi de acces altii: %s\n\n",
                              name, header.height, header.width, image_info.st_size, image_info.st_uid, date, image_info.st_nlink, user, group, other);

    if (write(out_descr, output, output_size) < 0)
    {
        perror("Error writing to file\n");
        exit(EXIT_FAILURE);
    }
    free(user);
    free(group);
    free(other);
}

DIR *open_directory(char *path)
{
    DIR *temp = NULL;
    temp = opendir(path);
    if (temp == NULL)
    {
        perror("Error opening the directory\n");
        exit(EXIT_FAILURE);
    }
    return temp;
}

void close_directory(DIR *dir)
{
    if (closedir(dir) < 0)
    {
        perror("Errror closing the directory\n");
        exit(EXIT_FAILURE);
    }
}

int check_bmp(char *file_name)
{
    unsigned short int length = strlen(file_name);
    if (length < 4 || strcmp(file_name + length - 4, ".bmp") != 0)
        return 0;
    return 1;
}

void process_bmp(int bmp_descriptor, char *bmp_name)
{
    BMPHeader header = read_header(bmp_descriptor, header);
    print_stats_bmp(bmp_descriptor, output_descriptor, header, bmp_name);
}

void process_regular(char *reg_name, struct stat dirent_stat)
{
    char output[1000];

    char date[12];
    strftime(date, sizeof(date), "%d.%m.%Y", gmtime(&dirent_stat.st_mtime));

    char *user = get_permissions('u', dirent_stat.st_mode);
    char *group = get_permissions('g', dirent_stat.st_mode);
    char *other = get_permissions('o', dirent_stat.st_mode);

    int output_size = sprintf(output,
                              "nume fisier: %s\n"
                              "dimensiune: %ld\n"
                              "identificatorul utilizatorului: %d\n"
                              "timpul ultimei modificari: %s\n"
                              "contorul de legaturi: %ld\n"
                              "drepturi de acces user: %s\n"
                              "drepturi de acces grup: %s\n"
                              "drepturi de acces altii: %s\n\n",
                              reg_name, dirent_stat.st_size, dirent_stat.st_uid, date, dirent_stat.st_nlink, user, group, other);

    if (write(output_descriptor, output, output_size) < 0)
    {
        perror("Error writing to file\n");
        exit(EXIT_FAILURE);
    }
    free(user);
    free(group);
    free(other);
}

void print_directory(char *dir_name, struct stat dirent_stat)
{
    char output[1000];

    char *user = get_permissions('u', dirent_stat.st_mode);
    char *group = get_permissions('g', dirent_stat.st_mode);
    char *other = get_permissions('o', dirent_stat.st_mode);

    int output_size = sprintf(output,
                              "nume director: %s\n"
                              "identificatorul utilizatorului: %d\n"
                              "drepturi de acces user: %s\n"
                              "drepturi de acces grup: %s\n"
                              "drepturi de acces altii: %s\n\n",
                              dir_name, dirent_stat.st_uid, user, group, other);

    if (write(output_descriptor, output, output_size) < 0)
    {
        perror("Error writing to file\n");
        exit(EXIT_FAILURE);
    }
    free(user);
    free(group);
    free(other);
}

void print_link(char *link_name, struct stat dirent_stat, char *path)
{
    char output[1000];

    struct stat link_stat;
    if (lstat(path, &link_stat) < 0)
    {
        perror("Error reding link information");
        exit(EXIT_FAILURE);
    }

    char *user = get_permissions('u', dirent_stat.st_mode);
    char *group = get_permissions('g', dirent_stat.st_mode);
    char *other = get_permissions('o', dirent_stat.st_mode);

    int output_size = sprintf(output,
                              "nume legatura: %s\n"
                              "dimensiune: %ld\n"
                              "dimensiune fisier: %ld\n"
                              "drepturi de acces user: %s\n"
                              "drepturi de acces grup: %s\n"
                              "drepturi de acces altii: %s\n\n",
                              link_name, link_stat.st_size, dirent_stat.st_size, user, group, other);

    if (write(output_descriptor, output, output_size) < 0)
    {
        perror("Error writing to file\n");
        exit(EXIT_FAILURE);
    }
    free(user);
    free(group);
    free(other);
}

void process_dirent(struct dirent *d, char *arg)
{
    char path[500];
    struct stat dirent_stat;
    sprintf(path, "%s/%s", arg, d->d_name);

    if (stat(path, &dirent_stat) < 0)
    {
        perror("Error reading file information");
        exit(EXIT_FAILURE);
    }

    if (S_ISDIR(dirent_stat.st_mode))
    {
        print_directory(d->d_name, dirent_stat);
    }
    else if (d->d_type == DT_LNK)
    {
        print_link(d->d_name, dirent_stat, path);
    }
    else if (S_ISREG(dirent_stat.st_mode))
    {
        int is_bmp = check_bmp(d->d_name);
        int descriptor = open_file(path);
        if (is_bmp == 1)
            process_bmp(descriptor, d->d_name);
        else
            process_regular(d->d_name, dirent_stat);
        close(descriptor);
    }
}

void process_directory(DIR *dir, char *arg)
{
    struct dirent *d = NULL;
    while ((d = readdir(dir)) != NULL)
    {
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
            continue;
        process_dirent(d, arg);
    }
}

int main(int argc, char *argv[])
{
    usage(argc, argv);

    output_descriptor = open_txt();

    DIR *directory = open_directory(argv[1]);

    process_directory(directory, argv[1]);

    close_directory(directory);

    close(output_descriptor);

    return 0;
}
