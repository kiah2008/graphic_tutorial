#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/types.h>
#include <poll.h>

/**
 * mypopen will run the p_cmdstring as a shell command.
 *
 * mypopen will return 0 if succeed, and *p_pid will be the child process's pid. 
 * *p_stdout_fd will be the fd of the child process's standard output stream. 
 * And *p_stderr_fd will be the fd of the child process's standard error stream.
 *
 * If mypopen failed, p_pid, p_stdout_fd, p_stderr_fd are undifined and should 
 * be ignored.
 *
 * If succeed, p_stdout_fd and p_stderr_fd can be read the obtain the standard
 * output and error of the command. You must ensure to invoke mypclose to wait
 * for the exit of the child process.
 *
 */
static int mypopen(const char *p_cmdstring, int *p_pid, int *p_stdout_fd, int *p_stderr_fd)
{
    int rc = 0;
    int pid = 0;
    int out[2];
    int err[2];

    rc = pipe(out);
    if (rc < 0)
    {
        goto l_out;
    }

    rc = pipe(err);
    if (rc < 0)
    {
        goto l_error_err;
    }
    
    if ((pid = fork()) < 0) {
        rc = pid;
        goto l_error_fork;
    } else if (pid == 0) { /* child */
        close(0);
        close(out[0]);
        close(err[0]);
        if (out[1] != STDOUT_FILENO) {
            dup2(out[1], STDOUT_FILENO);
            close(out[1]);
        }
        if (err[1] != STDERR_FILENO) {
            dup2(err[1], STDERR_FILENO);
            close(err[1]);
        }

        execl("/bin/sh", "sh", "-c", p_cmdstring, NULL);
        _exit(127);
    }

    /* parent continues... */
    close(out[1]);
    close(err[1]);

    *p_pid = pid;
    *p_stdout_fd = out[0];
    *p_stderr_fd = err[0];

l_out:
    return rc;

l_error_fork:
    close(err[0]);
    close(err[1]);

l_error_err:
    close(out[0]);
    close(out[1]);
    goto l_out;
}

static int mypclose(int pid, int stdout_fd, int stderr_fd)
{
    int status = 0;

    close(stdout_fd);
    close(stderr_fd);
    waitpid(pid, &status, 0);

    return status >> 8;
}

static int read_output(int fd, char **pp_buf, int *p_offset, 
        int *p_size)
{
    int rc = 0;
    int offset = *p_offset;
    int size = *p_size;
    char *p_buf = *pp_buf;
    char *p_new_buf = NULL;
    const int buf_unit = 1024;
    
    if (p_buf == NULL)
    {
        p_buf = malloc(buf_unit);
        if (p_buf == NULL)
        {
            rc = -1;
            goto l_out;
        }
        size = buf_unit;
        offset = 0;
    }
    if (*p_offset == *p_size)
    {
        // Expand the buf.
        p_new_buf = realloc(p_buf, size + buf_unit);
        if (p_new_buf == NULL)
        {
            rc = -1;
            goto l_fail;
        }
        p_buf = p_new_buf;
        size += buf_unit;
    }
    rc = read(fd, p_buf + offset, size - offset);
    if (rc < 0)
    {
        goto l_fail;
    }
    offset += rc;

l_out:
    *pp_buf = p_buf;
    *p_offset = offset;
    *p_size = size;
    return rc;

l_fail:
    free(p_buf);
    p_buf = NULL;
    size = 0;
    offset = 0;
    goto l_out;
}

static int read_all_output(
        int stdout_fd, 
        int stderr_fd, 
        char **pp_stdout, 
        int *p_stdout_size,
        char **pp_stderr,
        int *p_stderr_size)
{
    int rc = 0;
    fd_set rfds;
    struct pollfd fds[2];
    char *p_stdout_buf = NULL;
    char *p_stderr_buf = NULL;
    int stdout_offset = 0;
    int stderr_offset = 0;
    int stdout_buf_size = 0;
    int stderr_buf_size = 0;
    int maxfd = 0;
    int stdout_finished = 0;
    int stderr_finished = 0;

    fds[0].fd = stdout_fd;
    fds[0].events = POLLIN | POLLPRI;
    fds[1].fd = stderr_fd;
    fds[1].events = POLLIN | POLLPRI;
    while (!stdout_finished || !stderr_finished)
    {
        
        rc = poll(fds, 2, -1);
        if (rc == -1)
        {
            rc = -errno;
            goto l_fail;
        }
        if (rc == 0)
        {
            // No fd is ready.
            break;
        }
        if ((fds[0].revents & POLLIN) || (fds[0].revents & POLLPRI) || (fds[0].revents & POLLHUP))
        {
            // Read standard output.
            if (NULL != pp_stdout)
            {
                rc = read_output(stdout_fd, &p_stdout_buf, &stdout_offset, &stdout_buf_size);
            }
            else
            {
                rc = read_output(stdout_fd, NULL, &stdout_offset, &stdout_buf_size);
            }
            if (rc < 0)
            {
                goto l_fail;
            }
            if (rc == 0)
            {
                stdout_finished = 1;
            }
        }
        else if (fds[0].revents & POLLERR)
        {
            rc = -1;
            goto l_fail;
        }
        
        if ((fds[1].revents & POLLIN) || (fds[1].revents & POLLPRI) || (fds[1].revents & POLLHUP))
        {
            // Read error output.
            if (NULL != pp_stdout)
            {
                rc = read_output(stderr_fd, &p_stderr_buf, &stderr_offset, &stderr_buf_size);
            }
            else
            {
                rc = read_output(stderr_fd, NULL, &stderr_offset, &stderr_buf_size);
            }
            if (rc < 0)
            {
                goto l_fail;
            }
            if (rc == 0)
            {
                stderr_finished = 1;
            }
        }
        else if (fds[1].revents & POLLERR)
        {
            rc = -1;
            goto l_fail;
        }
    }
    
l_out:
    if (NULL != pp_stdout)
    {
        *pp_stdout = p_stdout_buf;
    }
    if (NULL != p_stdout_size)
    {
        *p_stdout_size = stdout_offset;
    }
    if (NULL != pp_stderr)
    {
        *pp_stderr = p_stderr_buf;
    }
    if (NULL != p_stderr_size)
    {
        *p_stderr_size = stderr_offset;
    }
    return rc;

l_fail:
    if (p_stdout_buf != NULL)
    {
        free(p_stdout_buf);
        p_stdout_buf = NULL;
        stdout_offset = 0;
    }
    if (p_stderr_buf != NULL)
    {
        free(p_stderr_buf);
        p_stderr_buf = NULL;
        stderr_offset = 0;
    }
    goto l_out;
}

/**
 * shellcmd will run the p_cmd as a shell command.
 * If return code is 0, then *p_exitcode is the exit status of the shell command,
 * *pp_stdout is the standard output buffer, *p_stdout_size is the size of the
 * standard output buffer, pp_stderr is the error output buffer, *p_stderr_size 
 * is the size of the error output buffer.
 *
 * If failed, all the output parameters' values are undefined and should 
 * be ignored.
 *
 * If succeed, *pp_stdout and *pp_stderr must be freed manually.
 *
 */
int runCmd(char *p_cmd, int *p_exitcode, char **pp_stdout, 
        int *p_stdout_size, char **pp_stderr, int *p_stderr_size)
{
    int rc = 0;
    int pid = 0;
    int stdout_fd = 0;
    int stderr_fd = 0;

    rc = mypopen(p_cmd, &pid, &stdout_fd, &stderr_fd);
    if (rc < 0)
    {
        return rc;
    }

    rc = read_all_output(
        stdout_fd,
        stderr_fd,
        pp_stdout,
        p_stdout_size,
        pp_stderr,
        p_stderr_size);

    *p_exitcode = mypclose(pid, stdout_fd, stderr_fd);
    
    return rc;
}
