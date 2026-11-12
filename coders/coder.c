/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 18:09:17 by tkern             #+#    #+#             */
/*   Updated: 2026/11/09 16:19:08 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Like sleep_ms, but returns early when the simulation stops */
static void	coder_sleep(t_sim *sim, long ms)
{
	long	end;

	end = get_time_ms() + ms;
	while (get_time_ms() < end && !sim_stopped(sim))
		usleep(500);
}

/*
** Odd coders take left first, even coders take right first. A deadlock
** needs every coder holding one dongle and waiting in the same
** direction, which this alternation makes impossible. It also avoids
** long chains of coders each holding one dongle.
** With one coder there is only one dongle, so they wait for the burnout
** The two dongle lines are printed later, together with "is compiling"
*/
static int	take_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	first = coder->left;
	second = coder->right;
	if (coder->id % 2 == 0)
	{
		first = coder->right;
		second = coder->left;
	}
	if (acquire_dongle(coder, first) < 0)
		return (-1);
	if (first == second)
		log_state(coder, MSG_DONGLE);
	if (first == second || acquire_dongle(coder, second) < 0)
	{
		while (first == second && !sim_stopped(coder->sim))
			usleep(500);
		release_dongle(first, coder->sim->config.dongle_cooldown);
		return (-1);
	}
	return (0);
}

/* Returns -1 if the simulation stopped while waiting for dongles. */
static int	compile(t_coder *coder)
{
	t_config	*cfg;

	cfg = &coder->sim->config;
	if (take_dongles(coder) < 0)
		return (-1);
	pthread_mutex_lock(&coder->mutex);
	coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&coder->mutex);
	log_compile(coder);
	coder_sleep(coder->sim, cfg->time_to_compile);
	pthread_mutex_lock(&coder->mutex);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->mutex);
	release_dongle(coder->left, cfg->dongle_cooldown);
	release_dongle(coder->right, cfg->dongle_cooldown);
	return (0);
}

/* Thread routine: compile, debug, refactor, repeat until stopped. */
void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!sim_stopped(coder->sim))
	{
		if (compile(coder) < 0)
			break ;
		log_state(coder, MSG_DEBUGGING);
		coder_sleep(coder->sim, coder->sim->config.time_to_debug);
		log_state(coder, MSG_REFACTOR);
		coder_sleep(coder->sim, coder->sim->config.time_to_refactor);
	}
	return (NULL);
}
