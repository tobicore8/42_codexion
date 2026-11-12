/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 17:55:36 by tkern             #+#    #+#             */
/*   Updated: 2026/11/03 18:48:30 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Priority of a request: smaller key = served first. */
static long	make_key(t_coder *coder, t_dongle *dongle)
{
	if (coder->sim->config.scheduler == FIFO)
		return (dongle->next_ticket++);
	return (coder->last_compile_start + coder->sim->config.time_to_burnout);
}

static int	is_my_turn(t_coder *coder, t_dongle *dongle)
{
	t_request	top;

	if (heap_peek(&dongle->queue, &top) < 0)
		return (0);
	return (top.coder_id == coder->id);
}

/*
** Waits (dongle mutex held) until the coder is first in the queue,
** the dongle is free and its cooldown is over, or the sim stopped
** If only the cooldown is missing, sleep exactly until it ends
*/
static void	wait_turn(t_coder *coder, t_dongle *dongle)
{
	struct timespec	until;

	while (!sim_stopped(coder->sim))
	{
		if (is_my_turn(coder, dongle) && !dongle->in_use)
		{
			if (get_time_ms() >= dongle->available_at)
				return ;
			until.tv_sec = dongle->available_at / 1000;
			until.tv_nsec = (dongle->available_at % 1000) * 1000000;
			pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &until);
		}
		else
			pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
}

/* Returns 0 when the dongle is taken, -1 if the simulation stopped. */
int	acquire_dongle(t_coder *coder, t_dongle *dongle)
{
	t_request	req;

	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->queue, coder->id, make_key(coder, dongle));
	wait_turn(coder, dongle);
	if (sim_stopped(coder->sim))
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (-1);
	}
	heap_pop(&dongle->queue, &req);
	dongle->in_use = 1;
	pthread_mutex_unlock(&dongle->mutex);
	return (0);
}

/* Puts the dongle back and starts its cooldown. */
void	release_dongle(t_dongle *dongle, long cooldown)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->in_use = 0;
	dongle->available_at = get_time_ms() + cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
