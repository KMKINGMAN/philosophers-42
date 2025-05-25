/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosopher_routine.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 09:42:47 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/25 15:59:09 by mkurkar          ###   ########.fr       */
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
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Deadlock Prevention Strategy:                   │
 * │                                                 │
 * │ Even-numbered philosophers:                     │
 * │   First take right fork, then left fork         │
 * │                                                 │
 * │ Odd-numbered philosophers:                      │
 * │   First take left fork, then right fork         │
 * │                                                 │
 * │ This breaks the circular wait condition         │
 * └─────────────────────────────────────────────────┘
 */
static void	setup_forks(t_philo *philo, t_fork **first, t_fork **second)
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
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ One Philosopher Case:                           │
 * │                                                 │
 * │ With only one philosopher and one fork, the     │
 * │ philosopher will:                               │
 * │ 1. Pick up the only fork                        │
 * │ 2. Can never pick up a second fork              │
 * │ 3. Wait until time_to_die elapses               │
 * │ 4. Die of starvation                            │
 * │                                                 │
 * │ This is the expected behavior per the problem   │
 * └─────────────────────────────────────────────────┘
 */
static int	handle_single_philo(t_philo *philo, t_fork *first)
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
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Meal Status Flow:                               │
 * │                                                 │
 * │ When is_eating = 1 (Start eating):              │
 * │ ┌──────────────────────────────┐                │
 * │ │ eating = 1                   │                │
 * │ │ last_meal_time = current time│                │
 * │ └──────────────────────────────┘                │
 * │                                                 │
 * │ When is_eating = 0 (Finish eating):             │
 * │ ┌──────────────────────────────┐                │
 * │ │ eating = 0                   │                │
 * │ │ meals_eaten++                │                │
 * │ └──────────────────────────────┘                │
 * │                                                 │
 * │ Protected by mutex to ensure thread safety      │
 * └─────────────────────────────────────────────────┘
 */
static void	update_meal_status(t_philo *philo, int is_eating)
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
 * @name philo_eat
 * @brief Handles the eating action of a philosopher
 *
 * @param philo Pointer to philosopher structure
 * @return int SUCCESS if eating completed, FAILURE otherwise
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Eating Process:                                 │
 * │                                                 │
 * │ 1. Set up forks (first and second)              │
 * │ 2. Pick up first fork                           │
 * │ 3. Handle single philosopher case               │
 * │ 4. Pick up second fork                          │
 * │ 5. Update meal status to eating                 │
 * │ 6. Sleep for time_to_eat milliseconds           │
 * │ 7. Update meal status to not eating             │
 * │ 8. Release both forks in reverse order          │
 * │                                                 │
 * │ Fork acquisition follows deadlock prevention    │
 * └─────────────────────────────────────────────────┘
 */
int	philo_eat(t_philo *philo)
{
	t_fork	*first_fork;
	t_fork	*second_fork;

	setup_forks(philo, &first_fork, &second_fork);
	pthread_mutex_lock(&first_fork->mutex);
	print_status(philo, "has taken a fork");
	if (handle_single_philo(philo, first_fork))
		return (FAILURE);
	pthread_mutex_lock(&second_fork->mutex);
	print_status(philo, "has taken a fork");
	update_meal_status(philo, 1);
	print_status(philo, "is eating");
	precise_sleep(philo->data->time_to_eat);
	update_meal_status(philo, 0);
	pthread_mutex_unlock(&second_fork->mutex);
	pthread_mutex_unlock(&first_fork->mutex);
	return (SUCCESS);
}

/**
 * @name philosopher_routine
 * @brief The main function for each philosopher thread
 *
 * @param arg Void pointer to philosopher structure
 * @return void* NULL
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Philosopher Lifecycle:                          │
 * │                                                 │
 * │ 1. Wait for all threads to be ready             │
 * │ 2. Even-numbered philosophers delay briefly     │
 * │    to prevent deadlocks                         │
 * │ 3. Set initial meal time                        │
 * │ 4. Enter main lifecycle loop:                   │
 * │    ┌────────────┐                               │
 * │    │ Check if   │                               │
 * │    │ simulation │                               │
 * │    │ has stopped│                               │
 * │    └─────┬──────┘                               │
 * │          ↓                                      │
 * │    ┌────────────┐                               │
 * │    │   Eat      │                               │
 * │    └─────┬──────┘                               │
 * │          ↓                                      │
 * │    ┌────────────┐                               │
 * │    │   Sleep    │                               │
 * │    └─────┬──────┘                               │
 * │          ↓                                      │
 * │    ┌────────────┐                               │
 * │    │   Think    │                               │
 * │    └────────────┘                               │
 * │                                                 │
 * │ Exits if simulation stops or action fails       │
 * └─────────────────────────────────────────────────┘
 */
void	*philosopher_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	wait_for_all_threads(philo);
	if (philo->id % 2 == 0)
		usleep(1000);
	pthread_mutex_lock(&philo->meal_mutex);
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->meal_mutex);
	while (1)
	{
		pthread_mutex_lock(&philo->data->state_mutex);
		if (philo->data->simulation_stop)
		{
			pthread_mutex_unlock(&philo->data->state_mutex);
			break ;
		}
		pthread_mutex_unlock(&philo->data->state_mutex);
		if (philo_eat(philo) == FAILURE || philo_sleep(philo) == FAILURE
			|| philo_think(philo) == FAILURE)
			break ;
	}
	return (NULL);
}
