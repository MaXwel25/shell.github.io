#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <mntent.h>

#define BUFFER_SIZE 1024
#define HIST_FILE "history.txt"
#define HIST_LIMIT 50 // Ограничение на количество хранимых команд

// Функция для сохранения команды в файл истории
void save_command(const char *command) {
    int fd = open(HIST_FILE, O_APPEND | O_WRONLY | O_CREAT, 0644);
    if (fd != -1) {
        write(fd, command, strlen(command));
        write(fd, "\n", 1);
        close(fd);
    }
}

void print_env_var(const char *var) {
    char *value = getenv(var);
    if (value) {
        printf("%s: %s\n", var, value);
    } else {
        printf("Переменная окружения %s не найдена.\n", var);
    }
}

void execute_binary(const char *binary) {
    pid_t pid = fork();
    if (pid == 0) {
        // Дочерний процесс
        execlp(binary, binary, NULL);
        perror("Ошибка выполнения бинарника");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Ошибка создания процесса");
    } else {
        // Родительский процесс
        wait(NULL); // Ждать завершения дочернего процесса
    }
}

// Функция для печати введенной строки
void print_input(const char *input) {
    printf("%s\n", input);
}

// Функция для циклической печати строки
void print_loop(const char *input) {
    while (1) {
        printf("%s\n", input);
        char c = getchar();
        if (c == EOF) { // Ctrl+D
            break;
        }
    }
}

// Функция для показа списка команд
void show_help() {
    printf("Доступные команды:\n");
    printf("1. print_once");
    printf("2. print_loop (выход по Ctrl+D)\n");
    printf("3. echo\n");
    printf("4. history\n");
    printf("5. exit или \\q - Выход\n");
    printf("6. help\n");
    printf("7. \\e <*> \n");
    printf("8. <бинарник> - Выполнить указанный бинарник.\n");
    printf("9. \\l <путь> - Получить информацию о разделе в системе.\n");
    printf("10. \\cron - Подключить VFS в /tmp/vfs.\n");
    printf("11. \\mem <procid> - Получить дамп памяти процесса.\n");
    printf("12. По сигналу SIGHUP вывести \"Configuration reloaded\".\n");

}

// Функция для показа истории команд
void show_history() {
    FILE *file = fopen(HIST_FILE, "r");
    if (file == NULL) {
        printf("История команд пуста.\n");
        return;
    }

    char line[BUFFER_SIZE];
    int count = 0;

    printf("История команд:\n");
    while (fgets(line, sizeof(line), file) != NULL && count < HIST_LIMIT) {
        printf("%d: %s", count + 1, line);
        count++;
    }
    
    fclose(file);
}

void sighup_handler() {
	printf("Configuration reloaded.\n");
}

void sigint_handler() {
	printf("\nВыход из программы.\n");
	exit(1);
}

void dump_memory(pid_t pid) {
    char filename[BUFFER_SIZE];
    snprintf(filename, sizeof(filename), "/proc/%d/mem", pid);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла памяти");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    printf("Дамп памяти процесса %d:\n", pid);
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        // Выводим содержимое памяти в шестнадцатеричном формате
        for (ssize_t i = 0; i < bytes_read; i++) {
            printf("%02x ", (unsigned char)buffer[i]);
        }
        printf("\n");
    }

    if (bytes_read == -1) {
        perror("Ошибка чтения файла памяти");
    }

    close(fd);
}

void connect_vfs() {
    printf("Подключаем VFS...\n");
    if (mkdir("/tmp/vfs", 0755) == -1) {
        perror("Ошибка создания директории для VFS");
    } else {
        printf("VFS подключен в /tmp/vfs.\n");
    }
}

void list_partitions() {
    FILE *file;
    struct mntent *mnt;

    file = setmntent("/proc/mounts", "r");
    if (file == NULL) {
        perror("Ошибка открытия файла /proc/mounts");
        return;
    }

    printf("Список разделов:\n");
    while ((mnt = getmntent(file)) != NULL) {
        printf("Файловая система: %s, Точка монтирования: %s, Тип: %s\n",
               mnt->mnt_type, mnt->mnt_dir, mnt->mnt_fsname);
    }

    endmntent(file);
}

int main() {

	signal(SIGINT, sigint_handler);
    signal(SIGHUP, sighup_handler);

    char input[BUFFER_SIZE];

    while (1) {
        printf("Введите команду: ");
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            break; // Выход при ошибке ввода
        }

        input[strcspn(input, "\n")] = '\0'; // Удаляем символ новой строки
        save_command(input); // Сохраняем команду в истории

        if (strcmp(input, "exit") == 0 || strcmp(input, "\\q") == 0) {
            break;
        } else if (strcmp(input, "help") == 0) {
            show_help();
        } else if (strcmp(input, "history") == 0) {
            show_history();
        } else if (strcmp(input, "print_once") == 0) {
            printf("Введите строку: ");
            fgets(input, BUFFER_SIZE, stdin);
            print_input(input);
        } else if (strcmp(input, "print_loop") == 0) {
            printf("Введите строку: ");
            fgets(input, BUFFER_SIZE, stdin);
            print_loop(input);
        } else if (strncmp(input, "echo ", 5) == 0) {
            printf("%s\n", input + 5);
        } else if (strncmp(input, "./", 2) == 0) {
            execute_binary(input);
        } else if (strncmp(input, "\\e ", 3) == 0) {
            print_env_var(input + 3);
        } else if (strcmp(input, "\\l") == 0) {
            list_partitions();
        } else if (strcmp(input, "\\cron") == 0) {
            connect_vfs();
        } else if (strncmp(input, "\\mem ", 5) == 0) {
            pid_t pid = atoi(input + 5);
            dump_memory(pid);
        } else {
            printf("Неизвестная команда: %s\n", input);
        }
    }

    printf("Выход из программы\n");
    return 0;
}
