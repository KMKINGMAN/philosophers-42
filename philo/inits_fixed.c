/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inits_fixed.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 15:56:27 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/25 15:50:06 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/**
 * @name init_data
 * @brief Initializes the main data structure with program arguments
 *
 * @param data Pointer to the main data structure
 * @param argc Argument count from main
 * @param argv Argument values from main
 * @return int SUCCESS if initialization successful, FAILURE otherwise
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Arguments:                                      │
 * │                                                 │
 * │ argv[1]: number_of_philosophers                 │
 * │ argv[2]: time_to_die (ms)                       │
 * │ argv[3]: time_to_eat (ms)                       │
 * │ argv[4]: time_to_sleep (ms)                     │
 * │ argv[5]: [optional] number_of_times_to_eat      │
 * │                                                 │
 * │ Note: All values must be positive integers      │
 * └─────────────────────────────────────────────────┘
 */
int	init_data(t_data *data, int argc, char **argv)
{
	if (argc < 5 || argc > 6)
		return (printf("Error: Invalid number of arguments\n"), FAILURE);
	data->num_philosophers = ft_atoi(argv[1]);
	data->time_to_die = ft_atoi(argv[2]);
	data->time_to_eat = ft_atoi(argv[3]);
	data->time_to_sleep = ft_atoi(argv[4]);
	data->must_eat_count = -1;
	if (argc == 6)
		data->must_eat_count = ft_atoi(argv[5]);
	if (data->num_philosophers <= 0 || data->time_to_die <= 0
		|| data->time_to_eat <= 0 || data->time_to_sleep <= 0 || (argc == 6
			&& data->must_eat_count <= 0))
		return (printf("Error: Invalid arguments\n"), FAILURE);
	data->print_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	data->state_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	return (SUCCESS);
}

/**
 * @name init_forks
 * @brief Initializes the forks (mutexes) for philosophers
 *
 * @param data Pointer to the main data structure
 * @return int SUCCESS if initialization successful, FAILURE otherwise
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Fork Initialization Process:                    │
 * │                                                 │
 * │ 1. Allocate memory for all forks                │
 * │ 2. Initialize each fork's mutex                 │
 * │ 3. Assign unique ID to each fork                │
 * │                                                 │
 * │ Fork IDs: 0, 1, 2, ..., (num_philosophers-1)    │
 * └─────────────────────────────────────────────────┘
 */
int	init_forks(t_data *data)
{
	int	i;

	i = 0;
	data->forks = malloc(sizeof(t_fork) * data->num_philosophers);
	if (!data->forks)
		return (FAILURE);
	memset(data->forks, 0, sizeof(t_fork) * data->num_philosophers);
	while (i < data->num_philosophers)
	{
		data->forks[i].mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		data->forks[i].id = i;
		i++;
	}
	return (SUCCESS);
}

/**
 * @name init_philosophers
 * @brief Initializes the philosopher structures
 *
 * @param data Pointer to the main data structure
 * @return int SUCCESS if initialization successful, FAILURE otherwise
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Philosopher Initialization:                     │
 * │                                                 │
 * │ 1. Allocate memory for all philosophers         │
 * │ 2. For each philosopher i:                      │
 * │    - ID = i+1 (1-indexed for display)           │
 * │    - Left fork = fork[i]                        │
 * │    - Right fork = fork[(i+1) % total]           │
 * │                                                 │
 * │ Fork assignment prevents deadlocks by ensuring  │
 * │ all philosophers reach for forks in same order  │
 * └─────────────────────────────────────────────────┘
 */
int	init_philosophers(t_data *data)
{
	int	i;

	data->philosophers = malloc(sizeof(t_philo) * data->num_philosophers);
	if (!data->philosophers)
		return (FAILURE);
	memset(data->philosophers, 0, sizeof(t_philo) * data->num_philosophers);
	i = 0;
	while (i < data->num_philosophers)
	{
		data->philosophers[i].id = i + 1;
		data->philosophers[i].data = data;
		data->philosophers[i].left_fork = &data->forks[i];
		data->philosophers[i].right_fork = &data->forks[(i + 1)
			% data->num_philosophers];
		data->philosophers[i].meal_mutex = (pthread_mutex_t)\
PTHREAD_MUTEX_INITIALIZER;
		i++;
	}
	return (SUCCESS);
}
