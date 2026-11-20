/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 15:23:43 by tkern             #+#    #+#             */
/*   Updated: 2026/11/03 18:48:35 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	heap_swap(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

/*
** Returns 1 if a must be served before b. Smaller key first; on equal
** keys (same EDF deadline) the smaller coder id wins (tie-breaker)
*/
static int	heap_less(t_request *a, t_request *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	return (a->coder_id < b->coder_id);
}

/* Move the item at index i up until its parent is not bigger */
void	heap_sift_up(t_heap *heap, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!heap_less(&heap->data[i], &heap->data[parent]))
			return ;
		heap_swap(&heap->data[parent], &heap->data[i]);
		i = parent;
	}
}

/* Move the item at index i down until both children are not smaller. */
void	heap_sift_down(t_heap *heap, int i)
{
	int	smallest;
	int	left;
	int	right;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < heap->size
			&& heap_less(&heap->data[left], &heap->data[smallest]))
			smallest = left;
		if (right < heap->size
			&& heap_less(&heap->data[right], &heap->data[smallest]))
			smallest = right;
		if (smallest == i)
			return ;
		heap_swap(&heap->data[i], &heap->data[smallest]);
		i = smallest;
	}
}
