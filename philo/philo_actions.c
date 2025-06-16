/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_actions.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 15:45:00 by mkurkar           #+#    #+#             */
/*   Updated: 2025/06/16 10:31:33 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/**
 * @name setup_forks
 * @brief Sets up fork order based on philosopher ID to prevent deadlocks
 *
 * @param philo Pointer to philosopher structure
 * @param first Pointer to store the first fork to grab
 * @param second Pointer to store the second fork to grab
 */
void	setup_forks(t_philo *philo, t_fork **first, t_fork **second)
{
	if (philo->id % 2 == 0)
	{
		*first = philo->right_fork;
		*second = philo->left_fork;
	}
	else
	{
		*first = philo->left_fork;
		*second = philo->right_fork;
	}
}

/**
 * @name handle_single_philo
 * @brief Special case handler for when there is only one philosopher
 *
 * @param philo Pointer to philosopher structure
 * @param first Pointer to the first fork
 * @return int 1 if there's only one philosopher, 0 otherwise
 */
int	handle_single_philo(t_philo *philo, t_fork *first)
{
	if (philo->data->num_philosophers == 1)
	{
		pthread_mutex_unlock(&first->mutex);
		precise_sleep(philo->data->time_to_die);
		return (1);
	}
	return (0);
}

/**
 * @name update_meal_status
 * @brief Updates philosopher's meal status in a thread-safe manner
 *
 * @param philo Pointer to philosopher structure
 * @param is_eating 1 if starting to eat, 0 if finished eating
 */
void	update_meal_status(t_philo *philo, int is_eating)
{
	pthread_mutex_lock(&philo->meal_mutex);
	if (is_eating)
	{
		philo->eating = 1;
		philo->last_meal_time = get_time();
	}
	else
	{
		philo->eating = 0;
		philo->meals_eaten++;
	}
	pthread_mutex_unlock(&philo->meal_mutex);
}

/**
 * @name acquire_forks
 * @brief Acquires both forks for eating
 *
 * @param philo Pointer to philosopher structure
 * @param first_fork First fork to acquire
 * @param second_fork Second fork to acquire
 * @return int SUCCESS if both forks acquired, FAILURE otherwise
 */
static int	acquire_forks(t_philo *philo, t_fork *first_fork,
	t_fork *second_fork)
{
	pthread_mutex_lock(&first_fork->mutex);
	if (check_simulation_stop(philo))
	{
		pthread_mutex_unlock(&first_fork->mutex);
		return (FAILURE);
	}
	print_status(philo, "has taken a fork");
	if (handle_single_philo(philo, first_fork))
		return (FAILURE);
	pthread_mutex_lock(&second_fork->mutex);
	if (check_simulation_stop(philo))
	{
		pthread_mutex_unlock(&second_fork->mutex);
		pthread_mutex_unlock(&first_fork->mutex);
		return (FAILURE);
	}
	print_status(philo, "has taken a fork");
	return (SUCCESS);
}

/**
 * @name philo_eat
 * @brief Handles the eating action of a philosopher
 *
 * @param philo Pointer to philosopher structure
 * @return int SUCCESS if eating completed, FAILURE otherwise
 */
int	philo_eat(t_philo *philo)
{
	t_fork	*first_fork;
	t_fork	*second_fork;

	if (check_simulation_stop(philo))
		return (FAILURE);
	setup_forks(philo, &first_fork, &second_fork);
	if (acquire_forks(philo, first_fork, second_fork) == FAILURE)
		return (FAILURE);
	update_meal_status(philo, 1);
	print_status(philo, "is eating");
	if (interruptible_sleep(philo, philo->data->time_to_eat) == FAILURE)
	{
		update_meal_status(philo, 0);
		pthread_mutex_unlock(&second_fork->mutex);
		pthread_mutex_unlock(&first_fork->mutex);
		return (FAILURE);
	}
	update_meal_status(philo, 0);
	pthread_mutex_unlock(&second_fork->mutex);
	pthread_mutex_unlock(&first_fork->mutex);
	return (SUCCESS);
}
