#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

typedef struct
{
    unsigned int width;
    unsigned int height;
} BMPHeader;

void usage(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <fisier_intrare>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    unsigned short int length = strlen(argv[1]);
    if (length < 4 || strcmp(argv[1] + length - 4, ".bmp") != 0)
    {
        printf("Error: File must have extenstion .bmp\n");
        exit(EXIT_FAILURE);
    }
}

int open_image(char *name)
{
    int descriptor;
    if ((descriptor = open(name, O_RDONLY)) < 0)
    {
        printf("Error opening input file\n");
        exit(EXIT_FAILURE);
    }
    return descriptor;
}

BMPHeader read_header(int descriptor, BMPHeader header)
{
    if (lseek(descriptor, 18, SEEK_SET) < 0)
    {
        perror("Error moving cursor: ");
        exit(EXIT_FAILURE);
    }

    if (read(descriptor, &header, sizeof(header)) != sizeof(header))
    {
        printf("Error reading image header\n");
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
        printf("Error creating output file\n");
        exit(EXIT_FAILURE);
    }
    return descriptor;
}

char *get_permissions(char type, mode_t mode)
{
    char *temp = (char *)malloc(4);
    if (temp == NULL)
    {
        printf("Memory allocation error\n");
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

void print_statistics(int image_descr, int out_descr, BMPHeader header, char *name)
{
    char output[1000];
    struct stat image_info;

    if (fstat(image_descr, &image_info) < 0)
    {
        perror("Error reading file information");
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
		     "drepturi de acces altii: %s",
        name, header.height, header.width, image_info.st_size, image_info.st_uid, date, image_info.st_nlink, user, group, other
	);

    if (write(out_descr, output, output_size) < 0)
    {
        printf("Error writing to file\n");
        exit(EXIT_FAILURE);
    }
    free(user);
    free(group);
    free(other);
}

int main(int argc, char *argv[])
{
    usage(argc, argv);

    int f1 = open_image(argv[1]);
    BMPHeader header = read_header(f1, header);
    int f2 = open_txt();
    print_statistics(f1, f2, header, argv[1]);

    close(f1);
    close(f2);

    return 0;
}