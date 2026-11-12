/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 16:21:21 by tkern             #+#    #+#             */
/*   Updated: 2026/11/09 16:18:31 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Prints one state line, unless the simulation is already over. */
void	log_state(t_coder *coder, const char *message)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->log_mutex);
	if (!sim_stopped(sim))
		printf("%ld %d %s\n", get_elapsed_ms(sim), coder->id, message);
	pthread_mutex_unlock(&sim->log_mutex);
}

/*
** Prints both dongle lines and the compiling line in one block, so no
** other coder's line can appear between them.
*/
void	log_compile(t_coder *coder)
{
	t_sim	*sim;
	long	now;

	sim = coder->sim;
	pthread_mutex_lock(&sim->log_mutex);
	if (!sim_stopped(sim))
	{
		now = get_elapsed_ms(sim);
		printf("%ld %d %s\n", now, coder->id, MSG_DONGLE);
		printf("%ld %d %s\n", now, coder->id, MSG_DONGLE);
		printf("%ld %d %s\n", now, coder->id, MSG_COMPILING);
	}
	pthread_mutex_unlock(&sim->log_mutex);
}

/*
** Stops the simulation and prints the burnout line. Both happen under
** log_mutex, so no other line can appear after the burnout message.
** Then wakes every coder waiting for a dongle so they can quit.
*/
void	log_burnout(t_sim *sim, int coder_id)
{
	pthread_mutex_lock(&sim->log_mutex);
	sim_set_stop(sim);
	printf("%ld %d %s\n", get_elapsed_ms(sim), coder_id, MSG_BURNOUT);
	pthread_mutex_unlock(&sim->log_mutex);
	wake_all_dongles(sim);
}
