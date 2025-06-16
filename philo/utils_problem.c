/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_problem.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 15:55:48 by mkurkar           #+#    #+#             */
/*   Updated: 2025/06/16 10:31:32 by mkurkar          ###   ########.fr       */
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

/**
 * @name philo_sleep
 * @brief Handles the sleeping action of a philosopher
 *
 * @param philo Pointer to philosopher structure
 * @return int SUCCESS always
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Sleeping Process:                               │
 * │                                                 │
 * │ 1. Print sleeping status                        │
 * │ 2. Sleep for time_to_sleep milliseconds         │
 * │ 3. Return success                               │
 * │                                                 │
 * │ This simulates the philosopher resting          │
 * └─────────────────────────────────────────────────┘
 */
int	philo_sleep(t_philo *philo)
{
	if (check_simulation_stop(philo))
		return (FAILURE);
	print_status(philo, "is sleeping");
	if (interruptible_sleep(philo, philo->data->time_to_sleep) == FAILURE)
		return (FAILURE);
	return (SUCCESS);
}

/**
 * @name philo_think
 * @brief Handles the thinking action of a philosopher
 *
 * @param philo Pointer to philosopher structure
 * @return int SUCCESS always
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Thinking Process:                               │
 * │                                                 │
 * │ 1. Print thinking status                        │
 * │ 2. Brief pause (usleep 500 microseconds)        │
 * │ 3. Return success                               │
 * │                                                 │
 * │ This simulates the philosopher contemplating    │
 * └─────────────────────────────────────────────────┘
 */
int	philo_think(t_philo *philo)
{
	pthread_mutex_lock(&philo->data->state_mutex);
	if (philo->data->simulation_stop)
	{
		pthread_mutex_unlock(&philo->data->state_mutex);
		return (FAILURE);
	}
	pthread_mutex_unlock(&philo->data->state_mutex);
	print_status(philo, "is thinking");
	usleep(500);
	return (SUCCESS);
}

/**
 * @name check_simulation_stop
 * @brief Checks if simulation should stop
 *
 * @param philo Pointer to philosopher structure
 * @return int 1 if should stop, 0 otherwise
 */
int	check_simulation_stop(t_philo *philo)
{
	int	stop;

	pthread_mutex_lock(&philo->data->state_mutex);
	stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->state_mutex);
	return (stop);
}
