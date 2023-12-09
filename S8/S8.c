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
#include <sys/wait.h>

#define MAX_PID 100

typedef struct
{
    unsigned int width;
    unsigned int height;
} BMPHeader;

#pragma pack(1)
typedef struct bmpHeaderFull {
    char signature[2];
    int fileSize;
    int reserved;
    int dataOffset;
    int size;
    int width;
    int height;
    char planes[2];
    char bitCount[2];
    int compression;
    int xPixelPerM;
    int yPixelPerM;
    int colorsUsed;
    int colorsImportant;
} bmpHeaderFull;
#pragma pack()

int output_descriptor;
pid_t pid[MAX_PID];
int total_pid = 0;
int count_pid = 0;

void usage(int argc, char *argv[])
{
    if (argc != 3)
    {
        char error[100];
        sprintf(error, "Usage: %s <director_intrare> <director_iesire> \n", argv[0]);
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

int open_txt(char *name)
{
    int descriptor;
    if ((descriptor = open(name, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0)
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

void print_stats_bmp(int image_descr, char *out_path, BMPHeader header, char *name)
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
    int od = open_txt(out_path);
    if (write(od, output, output_size) < 0)
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

int process_bmp(int bmp_descriptor, char *bmp_name, char *out_path)
{
    BMPHeader header = read_header(bmp_descriptor, header);
    print_stats_bmp(bmp_descriptor, out_path, header, bmp_name);
    return 10;
}

int process_regular(char *reg_name, struct stat dirent_stat, char *out_path)
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
    int od = open_txt(out_path);
    if (write(od, output, output_size) < 0)
    {
        perror("Error writing to file\n");
        exit(EXIT_FAILURE);
    }
    free(user);
    free(group);
    free(other);
    return 8;
}

int print_directory(char *dir_name, struct stat dirent_stat, char *out_path)
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

    int od = open_txt(out_path);
    if (write(od, output, output_size) < 0)
    {
        perror("Error writing to file\n");
        exit(EXIT_FAILURE);
    }
    free(user);
    free(group);
    free(other);
    return 5;
}

int print_link(char *link_name, struct stat dirent_stat, char *path, char *out_path)
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
    int od = open_txt(out_path);
    if (write(od, output, output_size) < 0)
    {
        perror("Error writing to file\n");
        exit(EXIT_FAILURE);
    }
    free(user);
    free(group);
    free(other);
    return 6;
}

void check_new_process()
{
    if (total_pid == MAX_PID)
    {
        perror("Maximum number of processes allowed is reached");
        exit(EXIT_FAILURE);
    }
}

void bmp_conversion(char *path)
{
    int f1, bytesRead;
    char colors[3];
    struct bmpHeaderFull statsBmpHeader;

    if ((f1 = open(path, O_RDWR)) < 0) {
        perror("Error opening input file\n");
        exit(1);
    }

    if (read(f1, &statsBmpHeader, sizeof(bmpHeaderFull)) < 0) {
        perror("Error reading input file\n");
        exit(1);
    }

    lseek(f1, statsBmpHeader.dataOffset, SEEK_SET);

    while ((bytesRead = read(f1, colors, sizeof(colors))) > 0) {
        char grayscale = 0.299 * colors[0] + 0.587 * colors[1] + 0.114 * colors[2];
        memset(colors, grayscale, sizeof(colors));
        lseek(f1, -bytesRead, SEEK_CUR);
        write(f1, colors, sizeof(colors));
    }

    if (close(f1) < 0) {
        perror("Error closing file\n");
        exit(1);
    }
}

void process_dirent(struct dirent *d, char *inp, char *out)
{
    char path[500];
    char out_path[500];
    char *statistica = "_statistica.txt";
    struct stat dirent_stat;
    sprintf(path, "%s/%s", inp, d->d_name);
    sprintf(out_path, "%s/%s%s", out, d->d_name, statistica);

    check_new_process();

    if (stat(path, &dirent_stat) < 0)
    {
        perror("Error reading file information");
        exit(EXIT_FAILURE);
    }

    if (S_ISDIR(dirent_stat.st_mode) || d->d_type == DT_LNK || (S_ISREG(dirent_stat.st_mode) && check_bmp(d->d_name) == 0))
    {
        if ((pid[count_pid++] = fork()) < 0)
        {
            perror("Error creating process");
            exit(EXIT_FAILURE);
        }
        total_pid++;
        int current_pid = count_pid - 1;

        if (pid[current_pid] == 0)
        {
            int lines_count = 0;

            if (S_ISDIR(dirent_stat.st_mode))
            {
                lines_count = print_directory(d->d_name, dirent_stat, out_path);
            }
            else if (d->d_type == DT_LNK)
            {
                lines_count = print_link(d->d_name, dirent_stat, path, out_path);
            }
            else if (S_ISREG(dirent_stat.st_mode))
            {
                lines_count = process_regular(d->d_name, dirent_stat, out_path);
            }
            exit(lines_count);
        }
    }
    else if (check_bmp(d->d_name) == 1)
    {
        int lines_count = 0;
        for (int j = 0; j < 2; j++)
        {
            if ((pid[count_pid++] = fork()) < 0)
            {
                perror("Error creating process");
                exit(EXIT_FAILURE);
            }
            total_pid++;
            int current_pid = count_pid - 1;
            if(pid[current_pid] == 0)
            {
                if(j == 0)
                {
                    int descriptor = open_file(path);
                    lines_count = process_bmp(descriptor, d->d_name, out_path);
                    close(descriptor);
                    exit(lines_count);
                }
                else bmp_conversion(path);
                exit(lines_count);
            }
        }
    }
}

void process_directory(DIR *inp, char *i, char *out)
{
    struct dirent *d = NULL;
    while ((d = readdir(inp)) != NULL)
    {
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
            continue;
        process_dirent(d, i, out);
    }

    for (int i = 0; i < count_pid; i++)
    {
        int status;
        pid_t wait_pid = wait(&status);
        if (WIFEXITED(status))
            printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", wait_pid, WEXITSTATUS(status));
        else
            printf("Procesul %d s-a terminat anormal\n", wait_pid);

        if (WEXITSTATUS(status) > 0)
        {
            char output[1000];
            int size = sprintf(output, "%d\n", WEXITSTATUS(status));
            if (write(output_descriptor, output, size) < 0)
            {
                perror("Error writing to file\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    usage(argc, argv);

    output_descriptor = open_txt("statistica.txt");

    DIR *input_directory = open_directory(argv[1]);

    process_directory(input_directory, argv[1], argv[2]);

    close_directory(input_directory);

    close(output_descriptor);

    return 0;
}

