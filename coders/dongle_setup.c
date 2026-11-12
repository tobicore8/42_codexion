/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_setup.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 16:21:32 by tkern             #+#    #+#             */
/*   Updated: 2026/11/03 18:48:23 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_one_dongle(t_dongle *dongle, int capacity)
{
	if (pthread_mutex_init(&dongle->mutex, NULL) != 0)
		return (-1);
	if (pthread_cond_init(&dongle->cond, NULL) != 0)
	{
		pthread_mutex_destroy(&dongle->mutex);
		return (-1);
	}
	if (heap_init(&dongle->queue, capacity) < 0)
	{
		pthread_cond_destroy(&dongle->cond);
		pthread_mutex_destroy(&dongle->mutex);
		return (-1);
	}
	dongle->in_use = 0;
	dongle->available_at = 0;
	dongle->next_ticket = 0;
	return (0);
}

/* Cleans up only what was initialized if one dongle fails */
int	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (init_one_dongle(&sim->dongles[i],
				sim->config.number_of_coders) < 0)
		{
			destroy_dongles(sim, i);
			return (-1);
		}
		i++;
	}
	return (0);
}

/* Destroys the first count dongles. */
void	destroy_dongles(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		heap_free(&sim->dongles[i].queue);
		pthread_cond_destroy(&sim->dongles[i].cond);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		i++;
	}
}
