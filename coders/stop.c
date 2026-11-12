/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stop.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 20:01:40 by tkern             #+#    #+#             */
/*   Updated: 2026/11/09 16:18:10 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Lock order, to avoid deadlocks:
**   log_mutex -> stop_mutex
**   dongle mutex -> stop_mutex
** Never hold a dongle mutex and log_mutex at the same time!
*/
int	sim_stopped(t_sim *sim)
{
	int	stopped;

	pthread_mutex_lock(&sim->stop_mutex);
	stopped = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stopped);
}

void	sim_set_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
}

/*
** Wakes every coder blocked on a dongle. The dongle mutex is locked
** while broadcasting, so a coder cannot miss the wake-up between
** checking the stop flag and starting to wait
*/
void	wake_all_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

/* Stops without a burnout message: every coder compiled enough. */
void	sim_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->log_mutex);
	sim_set_stop(sim);
	pthread_mutex_unlock(&sim->log_mutex);
	wake_all_dongles(sim);
}
