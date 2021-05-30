#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>

#define INPUT_MAX_LENGTH 1024
#define MAX_ARGS 100
#define MAX_ARGS_LENGTH PATH_MAX

#define VERDE "\x1b[32m"
#define ROJO "\e[0;31m"
#define BLANCO "\x1b[37m"
#define AZUL "\x1b[34m"
#define reset "\e[0m"

/*
Consiste en poner el texto en "igualdad de condiciones"
*/
char *normalizar(char *string);

/*
Transformar texto en estructuras de datos
*/
void parse(char *linea, char *arg_ptr[MAX_ARGS]);

/*
Liberar memoria
*/
void free_args(char* arg[])
{
    for (int i = 0; arg[i] != NULL; i++)
    {
        free(arg[i]);
    }
}

int main(void)
{
    char comm_str[INPUT_MAX_LENGTH];
    char *arg_ptr[MAX_ARGS];
    char *dir;
    char dNow[1024];

    printf("\033[2J\033[1;1H");
    while (1)
    {
        fflush(NULL);
        getcwd(dNow, 1024);
        printf(VERDE "%s" BLANCO "@" AZUL "%s > " reset, getenv("USER"), dNow);

        if (!fgets(comm_str, MAX_INPUT, stdin))
            return 0;

        //Salto de linea
        if (comm_str[strlen(comm_str) - 1] == '\n')
            comm_str[strlen(comm_str) - 1] = '\0'; //final de sentencia.

        if (strlen(comm_str) == 0)
            continue;

        parse(comm_str, arg_ptr);
        normalizar((char *)arg_ptr[0]);

        //Salir del shell
        if (strcmp(arg_ptr[0], "exit") == 0)
        {
            free_args(arg_ptr);
            exit(0);
        }

        //Moverse entre directorios
        if (strcmp(arg_ptr[0], "cd") == 0)
        {
            dir = arg_ptr[1];
            if (dir == NULL)
            {
                dir = getenv("HOME");
                if (dir == NULL)
                {
                    perror(ROJO "Home directory not found" reset); //Si no se escribe nada junto al cd, vamos al Home Directory
                    dir = "./";
                }
            }

            if (chdir(dir) == -1)
                perror(ROJO "Directory not found" reset);

            continue;
        }

        pid_t pid;
        switch (pid = fork())
        {
        case -1:
            perror("ERROR");
            break;
        case 0:
            if (execvp(arg_ptr[0], arg_ptr) == -1)
            {
                perror(ROJO "Command not found" reset);
                exit(0);
            }
            break;
        default:
            wait(NULL);
            break;
        }

        free_args(arg_ptr);
    }

    exit(0);    

}

char *normalizar(char *string)
{
    unsigned char *i = (unsigned char *)string;

    while (*i)
    {
        *i = tolower((unsigned char)*i);
        i++;
    }

    return string;
}

void parse(char *linea, char *arg_ptr[MAX_ARGS])
{
    int start = 0;
    int end = 0;
    int i = 0;
    char *memo;
    while (start < strlen(linea))
    {
        for (; linea[end] != ' '; end++)
            ;
        arg_ptr[i] = malloc(end - start + 1);
        memcpy(arg_ptr[i], linea + start, end - start);
        arg_ptr[i][end - start + 1] = 0;
        i++;
        for (; linea[end] == ' ' && linea[end] != 0; end++)
            ;
        start = end;
    }

    arg_ptr[i] = NULL;
}
