/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 14:46:29 by tkern             #+#    #+#             */
/*   Updated: 2026/11/09 16:56:23 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

# define MSG_DONGLE		"has taken a dongle"
# define MSG_COMPILING	"is compiling"
# define MSG_DEBUGGING	"is debugging"
# define MSG_REFACTOR	"is refactoring"
# define MSG_BURNOUT	"burned out"

typedef struct s_sim	t_sim;

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

/*
** Read-only after parsing: threads can read it without locking
** Times are in milliseconds!
*/
typedef struct s_config
{
	int			number_of_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	int			number_of_compiles_required;
	long		dongle_cooldown;
	t_scheduler	scheduler;
}	t_config;

/*
** One waiting request in a dongle's queue.
** key = arrival order for FIFO, burnout deadline for EDF.
** The smallest key is served first.
*/
typedef struct s_request
{
	int		coder_id;
	long	key;
}	t_request;

/* Min-heap of requests (array based). */
typedef struct s_heap
{
	t_request	*data;
	int			size;
	int			capacity;
}	t_heap;

/*
** A dongle between two coders. Everything here is protected by mutex.
** available_at: timestamp when the cooldown is over.
** next_ticket: FIFO arrival counter, so two requests never share a key.
*/
typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	int				in_use;
	long			available_at;
	long			next_ticket;
	t_heap			queue;
}	t_dongle;

/*
** One coder = one thread.
** last_compile_start and compiles_done are read by the monitor,
** so they are protected by the coder's own mutex.
*/
typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	t_dongle		*left;
	t_dongle		*right;
	pthread_mutex_t	mutex;
	long			last_compile_start;
	int				compiles_done;
	t_sim			*sim;
}	t_coder;

/* Shared state of the whole simulation. */
struct s_sim
{
	t_config		config;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor;
	long			start_time;
	int				stop;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	log_mutex;
};

/* parse.c */
int		parse_args(int argc, char **argv, t_config *cfg);

/* time.c */
long	get_time_ms(void);
long	get_elapsed_ms(t_sim *sim);

/* log.c */
void	log_state(t_coder *coder, const char *message);
void	log_compile(t_coder *coder);
void	log_burnout(t_sim *sim, int coder_id);

/* stop.c */
int		sim_stopped(t_sim *sim);
void	sim_set_stop(t_sim *sim);
void	wake_all_dongles(t_sim *sim);
void	sim_stop(t_sim *sim);

/* dongle.c */
int		acquire_dongle(t_coder *coder, t_dongle *dongle);
void	release_dongle(t_dongle *dongle, long cooldown);

/* coder.c */
void	*coder_routine(void *arg);

/* monitor.c */
void	*monitor_routine(void *arg);

/* sim_setup.c */
int		sim_init(t_sim *sim);
void	sim_destroy(t_sim *sim);

/* dongle_setup.c */
int		init_dongles(t_sim *sim);
void	destroy_dongles(t_sim *sim, int count);

/* coder_setup.c */
int		init_coders(t_sim *sim);
void	destroy_coders(t_sim *sim, int count);

/* heap.c */
int		heap_init(t_heap *heap, int capacity);
void	heap_free(t_heap *heap);
int		heap_push(t_heap *heap, int coder_id, long key);
int		heap_pop(t_heap *heap, t_request *out);
int		heap_peek(t_heap *heap, t_request *out);

/* heap_utils.c  */
void	heap_swap(t_request *a, t_request *b);
void	heap_sift_up(t_heap *heap, int i);
void	heap_sift_down(t_heap *heap, int i);

#endif
