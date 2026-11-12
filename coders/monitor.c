/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 20:01:35 by tkern             #+#    #+#             */
/*   Updated: 2026/11/09 16:18:25 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Burnout: the coder did not start compiling within time_to_burnout ms
** since their last compile start (or the start of the simulation)
** Returns 1 if a coder burned out (and the simulation is stopped).
*/
static int	check_burnout(t_sim *sim)
{
	int		i;
	long	last;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		pthread_mutex_lock(&sim->coders[i].mutex);
		last = sim->coders[i].last_compile_start;
		pthread_mutex_unlock(&sim->coders[i].mutex);
		if (get_time_ms() - last > sim->config.time_to_burnout)
		{
			log_burnout(sim, sim->coders[i].id);
			return (1);
		}
		i++;
	}
	return (0);
}

/* Returns 1 if every coder compiled at least the required number. */
static int	all_done(t_sim *sim)
{
	int	i;
	int	done;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		pthread_mutex_lock(&sim->coders[i].mutex);
		done = sim->coders[i].compiles_done;
		pthread_mutex_unlock(&sim->coders[i].mutex);
		if (done < sim->config.number_of_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

/* Checks every 0.5 ms, so a burnout is printed well within 10 ms. */
void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (1)
	{
		if (check_burnout(sim))
			return (NULL);
		if (all_done(sim))
		{
			sim_stop(sim);
			return (NULL);
		}
		usleep(500);
	}
}
