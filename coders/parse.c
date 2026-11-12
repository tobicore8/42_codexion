/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 15:23:37 by tkern             #+#    #+#             */
/*   Updated: 2026/11/03 18:42:34 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <limits.h>

static void	print_usage(void)
{
	fprintf(stderr, "Usage: ./codexion number_of_coders time_to_burnout ");
	fprintf(stderr, "time_to_compile time_to_debug time_to_refactor ");
	fprintf(stderr, "number_of_compiles_required dongle_cooldown ");
	fprintf(stderr, "scheduler(fifo|edf)\n");
	fprintf(stderr, "All numbers must be positive integers.\n");
}

/* Digits only (no sign, no spaces), at most INT_MAX. */
static int	parse_number(const char *str, long *out)
{
	long	n;
	int		i;

	if (!str[0])
		return (-1);
	n = 0;
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (-1);
		n = n * 10 + (str[i] - '0');
		if (n > INT_MAX)
			return (-1);
		i++;
	}
	*out = n;
	return (0);
}

static int	parse_all_numbers(char **argv, long *values)
{
	int	i;

	i = 0;
	while (i < 7)
	{
		if (parse_number(argv[i + 1], &values[i]) < 0)
			return (-1);
		i++;
	}
	return (0);
}

static int	parse_scheduler(const char *str, t_scheduler *out)
{
	if (strcmp(str, "fifo") == 0)
		*out = FIFO;
	else if (strcmp(str, "edf") == 0)
		*out = EDF;
	else
		return (-1);
	return (0);
}

/* Returns 0 and fills cfg, or -1 after printing an error. */
int	parse_args(int argc, char **argv, t_config *cfg)
{
	long	values[7];

	if (argc != 9 || parse_all_numbers(argv, values) < 0
		|| parse_scheduler(argv[8], &cfg->scheduler) < 0
		|| values[0] < 1)
	{
		print_usage();
		return (-1);
	}
	cfg->number_of_coders = (int)values[0];
	cfg->time_to_burnout = values[1];
	cfg->time_to_compile = values[2];
	cfg->time_to_debug = values[3];
	cfg->time_to_refactor = values[4];
	cfg->number_of_compiles_required = (int)values[5];
	cfg->dongle_cooldown = values[6];
	return (0);
}
