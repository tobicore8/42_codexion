/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_setup.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 16:21:44 by tkern             #+#    #+#             */
/*   Updated: 2026/11/09 16:19:20 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Coder i (id i + 1) uses dongle i on the left and dongle i + 1 on the
** right. The last coder wraps around to dongle 0. With one coder,
** left and right are the same dongle!
*/
int	init_coders(t_sim *sim)
{
	int	i;
	int	n;

	n = sim->config.number_of_coders;
	i = 0;
	while (i < n)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].left = &sim->dongles[i];
		sim->coders[i].right = &sim->dongles[(i + 1) % n];
		sim->coders[i].last_compile_start = 0;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].sim = sim;
		if (pthread_mutex_init(&sim->coders[i].mutex, NULL) != 0)
		{
			destroy_coders(sim, i);
			return (-1);
		}
		i++;
	}
	return (0);
}

/* Destroys the mutexes of the first count coders. */
void	destroy_coders(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&sim->coders[i].mutex);
		i++;
	}
}
