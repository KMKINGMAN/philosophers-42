/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_problem.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 15:55:48 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/21 20:40:35 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/**
 * @name print_status
 * @brief Prints the status of a philosopher with timestamp
 *
 * @param philo Pointer to philosopher structure
 * @param status Status message to print
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Output Format: timestamp philosopher_id status  │
 * │                                                 │
 * │ Example: 123 2 is eating                        │
 * │          ↑   ↑ ↑                                │
 * │          |   | └─ Status message                │
 * │          |   └─── Philosopher ID                │
 * │          └─────── Timestamp (in milliseconds)   │
 * └─────────────────────────────────────────────────┘
 */
void	print_status(t_philo *philo, char *status)
{
	long long	current_time;

	pthread_mutex_lock(&philo->data->print_mutex);
	pthread_mutex_lock(&philo->data->state_mutex);
	if (!philo->data->simulation_stop)
	{
		current_time = get_time() - philo->data->start_time;
		printf("%lld %d %s\n", current_time, philo->id, status);
	}
	pthread_mutex_unlock(&philo->data->state_mutex);
	pthread_mutex_unlock(&philo->data->print_mutex);
}

/**
 * @name free_data
 * @brief Releases all allocated memory for the simulation
 *
 * @param data Pointer to the main data structure
 * 
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Memory Management Flow:                         │
 * │                                                 │
 * │ 1. Check if forks exist → Free them             │
 * │ 2. Check if philosophers exist → Free them      │
 * │                                                 │
 * │ Note: Sets pointers to NULL after freeing       │
 * │ to prevent use-after-free bugs.                 │
 * └─────────────────────────────────────────────────┘
 */
void	free_data(t_data *data)
{
	if (data->forks)
	{
		free(data->forks);
		data->forks = NULL;
	}
	if (data->philosophers)
	{
		free(data->philosophers);
		data->philosophers = NULL;
	}
}
