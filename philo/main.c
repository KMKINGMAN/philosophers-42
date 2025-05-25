/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 17:25:34 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/25 11:46:13 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/**
 * @name wait_for_all_threads
 * @brief Makes the philosopher thread wait until all threads are ready
 *
 * @param philo Pointer to philosopher structure
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Wait Mechanism:                                 │
 * │                                                 │
 * │ 1. Lock state mutex                             │
 * │ 2. Check if all threads are ready               │
 * │ 3. If not, unlock and briefly sleep             │
 * │ 4. Reacquire lock and check again               │
 * │ 5. Once all threads are ready, continue         │
 * │                                                 │
 * │ This ensures philosophers start simultaneously  │
 * └─────────────────────────────────────────────────┘
 */
void	wait_for_all_threads(t_philo *philo)
{
	pthread_mutex_lock(&philo->data->state_mutex);
	while (!philo->data->all_threads_ready)
	{
		pthread_mutex_unlock(&philo->data->state_mutex);
		usleep(100);
		pthread_mutex_lock(&philo->data->state_mutex);
	}
	pthread_mutex_unlock(&philo->data->state_mutex);
}

/* Thread creation */
/**
 * @name init_meal_times
 * @brief Initializes the last meal time for all philosophers
 *
 * @param data Pointer to main data structure
 * @return int SUCCESS always
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Meal Time Initialization:                       │
 * │                                                 │
 * │ 1. Iterate through all philosophers             │
 * │ 2. For each philosopher:                        │
 * │    a. Lock their meal mutex                     │
 * │    b. Set last_meal_time to simulation start    │
 * │    c. Unlock mutex                              │
 * │                                                 │
 * │ This ensures all philosophers start with a      │
 * │ synchronized last meal time value               │
 * └─────────────────────────────────────────────────┘
 */
static int	init_meal_times(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->num_philosophers)
	{
		pthread_mutex_lock(&data->philosophers[i].meal_mutex);
		data->philosophers[i].last_meal_time = data->start_time;
		pthread_mutex_unlock(&data->philosophers[i].meal_mutex);
		i++;
	}
	return (SUCCESS);
}

/**
 * @name init_philo_threads
 * @brief Creates threads for all philosophers and the monitor
 *
 * @param data Pointer to main data structure
 * @param monitor Pointer to monitor thread identifier
 * @return int SUCCESS if all threads created, FAILURE otherwise
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Thread Creation Process:                        │
 * │                                                 │
 * │ 1. For each philosopher:                        │
 * │    a. Create thread with philosopher_routine    │
 * │    b. Pass philosopher's data as argument       │
 * │    c. If creation fails, return FAILURE         │
 * │                                                 │
 * │ 2. Create monitor thread                        │
 * │    a. Run monitor_routine                       │
 * │    b. Pass main data structure as argument      │
 * │    c. If creation fails, return FAILURE         │
 * │                                                 │
 * │ No threads start their main work until all      │
 * │ threads have been created successfully          │
 * └─────────────────────────────────────────────────┘
 */
static int	init_philo_threads(t_data *data, pthread_t *monitor)
{
	int	i;

	i = 0;
	while (i < data->num_philosophers)
	{
		if (pthread_create(&data->philosophers[i].thread, NULL,
				philosopher_routine, &data->philosophers[i]) != 0)
			return (FAILURE);
		i++;
	}
	if (pthread_create(monitor, NULL, monitor_routine, data) != 0)
		return (FAILURE);
	return (SUCCESS);
}

/**
 * @name create_threads
 * @brief Orchestrates thread creation and synchronization
 *
 * @param data Pointer to main data structure
 * @return int SUCCESS if all operations completed, FAILURE otherwise
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Thread Creation and Management Flow:            │
 * │                                                 │
 * │ 1. Record simulation start time                 │
 * │ 2. Initialize meal times for all philosophers   │
 * │ 3. Create philosopher threads and monitor thread │
 * │ 4. Set the all_threads_ready flag to 1          │
 * │    (this releases waiting threads)              │
 * │                                                 │
 * │ 5. Wait for all philosopher threads to complete │
 * │ 6. Wait for monitor thread to complete          │
 * │                                                 │
 * │ This function manages the full lifecycle of     │
 * │ threads from creation to termination            │
 * └─────────────────────────────────────────────────┘
 */
int	create_threads(t_data *data)
{
	int			i;
	pthread_t	monitor;

	data->start_time = get_time();
	if (init_meal_times(data) == FAILURE)
		return (FAILURE);
	if (init_philo_threads(data, &monitor) == FAILURE)
		return (FAILURE);
	pthread_mutex_lock(&data->state_mutex);
	data->all_threads_ready = 1;
	pthread_mutex_unlock(&data->state_mutex);
	/* Wait for all philosopher threads to finish */
	i = 0;
	while (i < data->num_philosophers)
	{
		if (pthread_join(data->philosophers[i].thread, NULL) != 0)
			return (FAILURE);
		i++;
	}
	/* Wait for monitor thread */
	if (pthread_join(monitor, NULL) != 0)
		return (FAILURE);
	return (SUCCESS);
}

/**
 * @name main
 * @brief Entry point of the philosophers program
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line arguments
 * @return int 0 on success, 1 on failure
 *
 * Example:
 * ┌────────────────────────────────────────────────────┐
 * │ Program Execution Flow:                            │
 * │                                                    │
 * │ 1. Initialize data structure with zeros            │
 * │ 2. Parse and validate command-line arguments       │
 * │ 3. Initialize forks (mutexes)                      │
 * │ 4. Initialize philosopher structures               │
 * │ 5. Create and manage threads                       │
 * │ 6. Clean up resources                              │
 * │                                                    │
 * │ Program Arguments:                                 │
 * │ ./philo number_of_philosophers time_to_die         │
 * │        time_to_eat time_to_sleep                   │
 * │        [number_of_times_each_philosopher_must_eat] │
 * │                                                    │
 * │ Example: ./philo 5 800 200 200 7                   │
 * └────────────────────────────────────────────────────┘
 */
/* Main function */
int	main(int argc, char **argv)
{
	t_data data;

	memset(&data, 0, sizeof(t_data));
	if (init_data(&data, argc, argv) == FAILURE)
		return (1);
	if (init_forks(&data) == FAILURE)
	{
		free_data(&data);
		return (1);
	}
	if (init_philosophers(&data) == FAILURE)
	{
		free_data(&data);
		return (1);
	}
	if (create_threads(&data) == FAILURE)
	{
		printf("Error creating threads\n");
		free_data(&data);
		return (1);
	}
	free_data(&data);
	return (0);
}