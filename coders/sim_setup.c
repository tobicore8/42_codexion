/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_setup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 17:12:19 by tkern             #+#    #+#             */
/*   Updated: 2026/11/04 17:00:51 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	free_arrays(t_sim *sim)
{
	free(sim->coders);
	free(sim->dongles);
	sim->coders = NULL;
	sim->dongles = NULL;
}

static int	alloc_arrays(t_sim *sim)
{
	sim->coders = malloc(sizeof(t_coder) * sim->config.number_of_coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->config.number_of_coders);
	if (!sim->coders || !sim->dongles)
	{
		free_arrays(sim);
		return (-1);
	}
	return (0);
}

static int	init_sim_mutexes(t_sim *sim)
{
	if (pthread_mutex_init(&sim->stop_mutex, NULL) != 0)
		return (-1);
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->stop_mutex);
		return (-1);
	}
	return (0);
}

/* sim->config must already be filled. Returns 0, or -1 with no leaks */
int	sim_init(t_sim *sim)
{
	int	n;

	n = sim->config.number_of_coders;
	sim->stop = 0;
	sim->start_time = 0;
	if (alloc_arrays(sim) < 0)
		return (-1);
	if (init_dongles(sim) < 0 || init_coders(sim) < 0)
	{
		destroy_dongles(sim, n);
		free_arrays(sim);
		return (-1);
	}
	if (init_sim_mutexes(sim) < 0)
	{
		destroy_coders(sim, n);
		destroy_dongles(sim, n);
		free_arrays(sim);
		return (-1);
	}
	return (0);
}

/* Call after all threads are joined. */
void	sim_destroy(t_sim *sim)
{
	int	n;

	n = sim->config.number_of_coders;
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
	destroy_coders(sim, n);
	destroy_dongles(sim, n);
	free_arrays(sim);
}
