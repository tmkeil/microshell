/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shell.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkeil <tkeil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 16:52:49 by tkeil             #+#    #+#             */
/*   Updated: 2025/03/13 21:15:21 by tkeil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>

int ret = 0;

void ft_putstrs(char *s1, char *s2)
{
	if (s1)
	{
		while (*s1)
			write(2, s1++, 1);
	}
	if (s2)
	{
		while (*s2)
			write(2, s2++, 1);
	}
	write(2, "\n", 1);
}

int ft_ptr_len(char **ptr)
{
	int i = 0;

	while (ptr[i])
		i++;
	return (i);
}

char *ft_strdup(char *ptr)
{
	char *dup;

	int i = 0;
	while (ptr[i])
		i++;
	dup = malloc(sizeof(char) * (i + 1));
	if (!dup)
		return (NULL);
	i = 0;
	while (ptr[i])
	{
		dup[i] = ptr[i];
		i++;
	}
	dup[i] = '\0';
	return (dup);
}

int ft_create_cmds(char **av, char ***cmds, int j)
{
	*cmds = malloc(sizeof(char *) * (j + 1));
	if (!*cmds)
		return (0);
	(*cmds)[j] = NULL;
	while (j--)
	{
		(*cmds)[j] = ft_strdup(av[j]);
	}
	return (1);
}

void ft_clean_cmds(char **cmds)
{
	int i = 0;

	while (cmds[i])
		free(cmds[i++]);
	free(cmds);
	cmds = NULL;
}

void ft_cd(char **cmds)
{
	if (ft_ptr_len(cmds) != 2)
	{
		ret = 1;
		ft_putstrs("cd: too many arguments", NULL);
	}
	else if (chdir(cmds[1]) != 0)
	{
		ret = 1;
		ft_putstrs("cd: no such file or directory: ", cmds[1]);
	}
}

void ft_execute(char **cmds, char **env, int tmp_in)
{
	dup2(tmp_in, STDIN_FILENO);
	close(tmp_in);
	if (execve(cmds[0], cmds, env) == -1)
	{
		ft_putstrs("error: cannot execute ", cmds[0]);
		exit(1);
	}
}

int main(int ac, char **av, char **env)
{
	char **cmds;
	int fd[2], tmp_in = dup(STDIN_FILENO), i = 0, j = 0, pid;

	if (ac < 2)
		return (0);
	while (i + j + 1 < ac)
	{
		i += j + 1;
		j = 0;
		while (av[i + j] && strcmp(av[i + j], ";") && strcmp(av[i + j], "|"))
			j++;
		ft_create_cmds(&av[i], &cmds, j);
		if (!strcmp(cmds[0], "cd"))
			ft_cd(cmds);
		else if (j && (!av[i + j] || !strcmp(av[i + j], ";")))
		{
			if ((pid = fork()) == -1)
				return (ft_putstrs("error: fatal", NULL), 1);
			if (pid == 0)
			{
				ft_execute(cmds, env, tmp_in);
			}
			else
			{
				close(tmp_in);
				while (waitpid(-1, &ret, 0) != -1);
				tmp_in = dup(STDIN_FILENO);
				ft_clean_cmds(cmds);				
			}
		}
		else if (j && !strcmp(av[i + j], "|"))
		{
			if (pipe(fd) == -1)
				return (ft_putstrs("error: fatal", NULL), 1);
			if ((pid = fork()) == -1)
				return (ft_putstrs("error: fatal", NULL), close(fd[0]), close(fd[1]), 1);
			if (pid == 0)
			{
				dup2(fd[1], STDOUT_FILENO);
				close(fd[1]);
				close(fd[0]);
				ft_execute(cmds, env, tmp_in);
			}
			else
			{
				close(fd[1]);
				close(tmp_in);
				tmp_in = fd[0];
				ft_clean_cmds(cmds);
			}
		}
		else
			ft_clean_cmds(cmds);
	}
	close(tmp_in);
	return (ret);
}
