/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 14:46:41 by tkern             #+#    #+#             */
/*   Updated: 2026/11/03 18:48:10 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	join_coders(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

static int	abort_start(t_sim *sim, int count)
{
	sim_stop(sim);
	join_coders(sim, count);
	return (-1);
}

/*
** Every coder's burnout timer starts at the beginning of the simulation
** If a thread cannot be created, stop and join the ones already running
*/
static int	start_simulation(t_sim *sim)
{
	int	i;
	int	n;

	n = sim->config.number_of_coders;
	sim->start_time = get_time_ms();
	i = -1;
	while (++i < n)
		sim->coders[i].last_compile_start = sim->start_time;
	i = -1;
	while (++i < n)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
			return (abort_start(sim, i));
	}
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
		return (abort_start(sim, n));
	return (0);
}

int	main(int argc, char **argv)
{
	t_sim	sim;

	memset(&sim, 0, sizeof(sim));
	if (parse_args(argc, argv, &sim.config) < 0)
		return (1);
	if (sim_init(&sim) < 0)
	{
		fprintf(stderr, "Error: initialization failed\n");
		return (1);
	}
	if (start_simulation(&sim) < 0)
	{
		fprintf(stderr, "Error: could not create threads\n");
		sim_destroy(&sim);
		return (1);
	}
	pthread_join(sim.monitor, NULL);
	join_coders(&sim, sim.config.number_of_coders);
	sim_destroy(&sim);
	return (0);
}
