/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 17:12:07 by tkern             #+#    #+#             */
/*   Updated: 2026/11/18 15:15:58 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Current time in ms (absolute, from gettimeofday) */
long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/* ms since the simulation started: the timestamp used in the logs */
long	get_elapsed_ms(t_sim *sim)
{
	return (get_time_ms() - sim->start_time);
}
