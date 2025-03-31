#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// 方法1：使用dup2系统调用
void redirect_dup2(const char* cmd, const char* file) {
    int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    int saved_stdout = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);
    close(fd);

    system(cmd);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
}

// 方法2：使用freopen
void redirect_freopen(const char* cmd, const char* file) {
    FILE* f = freopen(file, "w", stdout);
    if (f == NULL) {
        perror("freopen");
        exit(EXIT_FAILURE);
    }

    system(cmd);

    freopen("/dev/tty", "w", stdout);
}

// 方法3：使用shell重定向
void redirect_system(const char* cmd, const char* file) {
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s > %s", cmd, file);
    system(full_cmd);
}

int main(int argc, char* argv[]) {
    // 最少需要5个参数（程序名 + -t + n + 命令 + 输出文件）
    if (argc < 5) {
        fprintf(stderr, "Error: Wrong number of arguments\n");
        fprintf(stderr, "Usage: %s -t [1|2|3] \"command\" output_file\n", argv[0]);
        fprintf(stderr, "Example: %s -t 2 \"ps aux\" ps_output.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    // 检查-t参数
    if (strcmp(argv[1], "-t") != 0) {
        fprintf(stderr, "Error: Missing -t option\n");
        return EXIT_FAILURE;
    }

    // 检查方法参数
    int method = atoi(argv[2]);
    if (method < 1 || method > 3) {
        fprintf(stderr, "Error: Method must be 1, 2 or 3\n");
        return EXIT_FAILURE;
    }

    // 合并命令参数（argv[3]到argv[argc-2]）
    char cmd[1024] = {0};
    for (int i = 3; i < argc - 1; i++) {
        strcat(cmd, argv[i]);
        if (i < argc - 2) {
            strcat(cmd, " ");
        }
    }

    const char* file = argv[argc - 1];

    switch (method) {
        case 1: redirect_dup2(cmd, file); break;
        case 2: redirect_freopen(cmd, file); break;
        case 3: redirect_system(cmd, file); break;
    }

    return EXIT_SUCCESS;
}
