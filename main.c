#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*****************
 shell内部函数声明
******************/
void cshell_loop(void);
char *cshell_read_line(void);
char **cshell_split_line(char*);
int cshell_launch(char **args);
int cshell_num_builtins();
int cshell_execute(char **args);

/*********************
 内置shell命令函数声明
**********************/
int cshell_cd(char **args);
int cshell_help(char **args);
int cshell_exit(char **args);

/************
 内置命令列表
************/
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

/****************
 内置命令对应函数 （将builtin_func声明为返回int的函数指针数组（指向char的指针））
****************/
int (*builtin_func[])(char **) = {
    &cshell_cd,
    &cshell_help,
    &cshell_exit
};

int main(int argc, char** argv)
{
    cshell_loop();
    
    return EXIT_SUCCESS;
}

void cshell_loop(void)
{
    char *line;
    char **args;
    int status;

    do{
        printf(">> ");
        line = cshell_read_line();
        args = cshell_split_line(line);
        status = cshell_execute(args);

        free(line);
        free(args);
    } while (status);
}

char *cshell_read_line(void)
{
#ifdef CSHELL_USE_STD_GETLINE
    char *line = NULL;
    ssize_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1)
    {
        if (feof(stdin))
        {
            exit(EXIT_SUCCESS);
        } else {
            perror("cshell: getline\n");
            exit(EXIT_FAILURE);
        }     
        // return line; //mistake
    }
    return line;  
#else
#define CSHELL_RL_BUFSIZE 1024
    int bufsize = CSHELL_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "cshell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    while (1) 
    {
        // 读取一个字符
        c = getchar();

        // 如果命中EOF, 将它替换为空字符串并且返回
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        } else{
            buffer[position] = c;
        }
        position++;
        
        // 如果超过缓冲区， 重新分配
        if (position >= bufsize)
        {
           bufsize += CSHELL_RL_BUFSIZE;
           buffer = realloc(buffer, bufsize);
           if (!buffer)
           {
                fprintf(stderr, "cshell: allocation error\n");
                exit(EXIT_FAILURE);
           }
        }
    }
#endif
}

#define CSHELL_TOK_BUFSIZE 64
#define CSHELL_TOK_DELIM " \t\r\n\a"
char **cshell_split_line(char *line)
{
    int bufsize = CSHELL_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;

    if (!tokens)
    {
        fprintf(stderr, "cshell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, CSHELL_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize){
            bufsize += CSHELL_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "cshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, CSHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;    
}

int cshell_launch(char **args)
{
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == 0){
       // 子进程执行
       if (execvp(args[0], args) == -1) {
            perror("cshell");
       }
       exit(EXIT_FAILURE);
    } else if (pid < 0){
        // fork Error
        perror("cshell");
    } else {
        // 父进程执行
        do {
            waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}

int cshell_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int cshell_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "cshell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0)
        {
            perror("cshell");
        }
    }
    return 1;    
}

int cshell_help(char **args)
{
    int i;
    printf("ForkShannon's cshell\n");
    printf("Type program names an arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for ( i = 0; i < cshell_num_builtins(); i++)
    {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int cshell_exit(char **args)
{
    return EXIT_SUCCESS;
}

int cshell_execute(char **args)
{
    int i;

    if(args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < cshell_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i]) (args);
        }
    }

    return cshell_launch(args);
}