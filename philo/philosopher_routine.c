/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosopher_routine.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 09:42:47 by mkurkar           #+#    #+#             */
/*   Updated: 2025/06/11 15:46:14 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/**
 * @name init_philosopher_state
 * @brief Initializes philosopher state before starting main loop
 *
 * @param philo Pointer to philosopher structure
 */
static void	init_philosopher_state(t_philo *philo)
{
	wait_for_all_threads(philo);
	if (philo->id % 2 == 0)
		usleep(1000);
	pthread_mutex_lock(&philo->meal_mutex);
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->meal_mutex);
}

/**
 * @name should_stop_simulation
 * @brief Checks if simulation should stop
 *
 * @param philo Pointer to philosopher structure
 * @return int 1 if should stop, 0 otherwise
 */
static int	should_stop_simulation(t_philo *philo)
{
	int	stop;

	pthread_mutex_lock(&philo->data->state_mutex);
	stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->state_mutex);
	return (stop);
}

/**
 * @name has_eaten_enough
 * @brief Checks if philosopher has eaten required number of meals
 *
 * @param philo Pointer to philosopher structure
 * @return int 1 if has eaten enough, 0 otherwise
 */
static int	has_eaten_enough(t_philo *philo)
{
	int	enough;

	if (philo->data->must_eat_count == -1)
		return (0);
	pthread_mutex_lock(&philo->meal_mutex);
	enough = (philo->meals_eaten >= philo->data->must_eat_count);
	pthread_mutex_unlock(&philo->meal_mutex);
	return (enough);
}

/**
 * @name philosopher_routine
 * @brief The main function for each philosopher thread
 *
 * @param arg Void pointer to philosopher structure
 * @return void* NULL
 */
void	*philosopher_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	init_philosopher_state(philo);
	while (1)
	{
		if (should_stop_simulation(philo))
			break ;
		if (has_eaten_enough(philo))
			break ;
		if (philo_eat(philo) == FAILURE || philo_sleep(philo) == FAILURE
			|| philo_think(philo) == FAILURE)
			break ;
	}
	return (NULL);
}
