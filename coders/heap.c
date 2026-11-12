/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkern <tkern@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 15:23:32 by tkern             #+#    #+#             */
/*   Updated: 2026/11/03 18:48:40 by tkern            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Returns 0 on success, -1 if malloc fails */
int	heap_init(t_heap *heap, int capacity)
{
	heap->data = malloc(sizeof(t_request) * capacity);
	if (!heap->data)
		return (-1);
	heap->size = 0;
	heap->capacity = capacity;
	return (0);
}

void	heap_free(t_heap *heap)
{
	free(heap->data);
	heap->data = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

/* Returns 0 on success, -1 if the heap is full. */
int	heap_push(t_heap *heap, int coder_id, long key)
{
	if (heap->size >= heap->capacity)
		return (-1);
	heap->data[heap->size].coder_id = coder_id;
	heap->data[heap->size].key = key;
	heap->size++;
	heap_sift_up(heap, heap->size - 1);
	return (0);
}

/* Removes the smallest key and stores it in out. -1 if empty. */
int	heap_pop(t_heap *heap, t_request *out)
{
	if (heap->size == 0)
		return (-1);
	*out = heap->data[0];
	heap->size--;
	heap->data[0] = heap->data[heap->size];
	heap_sift_down(heap, 0);
	return (0);
}

/* Reads the smallest key without removing it. -1 if empty. */
int	heap_peek(t_heap *heap, t_request *out)
{
	if (heap->size == 0)
		return (-1);
	*out = heap->data[0];
	return (0);
}
