/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosopher_monitor.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 11:31:46 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/25 15:54:51 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/**
 * @name check_philo_death
 * @brief Checks if a specific philosopher has died of starvation
 *
 * @param data Pointer to main data structure
 * @param philos Pointer to array of philosophers
 * @param i Index of philosopher to check
 * @return int 1 if the philosopher has died, 0 otherwise
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Death Check Algorithm:                          │
 * │                                                 │
 * │ 1. Lock philosopher's meal mutex                │
 * │ 2. Get current time                             │
 * │ 3. Calculate time since last meal:              │
 * │    current_time - last_meal_time                │
 * │                                                 │
 * │ 4. If not eating AND time since last meal       │
 * │    exceeds time_to_die:                         │
 * │    a. Unlock meal mutex                         │
 * │    b. Lock print and state mutexes              │
 * │    c. Stop the simulation                       │
 * │    d. Print death message                       │
 * │    e. Return 1 (philosopher died)               │
 * │                                                 │
 * │ 5. Otherwise, unlock and return 0 (alive)       │
 * └─────────────────────────────────────────────────┘
 */
static int	check_philo_death(t_data *data, t_philo *philos, int i)
{
	long long	current_time;

	pthread_mutex_lock(&philos[i].meal_mutex);
	current_time = get_time();
	if (!philos[i].eating && (current_time
			- philos[i].last_meal_time) >= data->time_to_die)
	{
		pthread_mutex_unlock(&philos[i].meal_mutex);
		pthread_mutex_lock(&data->print_mutex);
		pthread_mutex_lock(&data->state_mutex);
		data->simulation_stop = 1;
		printf("%lld %d died\n", get_time() - data->start_time, philos[i].id);
		pthread_mutex_unlock(&data->state_mutex);
		pthread_mutex_unlock(&data->print_mutex);
		return (1);
	}
	pthread_mutex_unlock(&philos[i].meal_mutex);
	return (0);
}

/**
 * @name check_all_philos
 * @brief Checks all philosophers to see if any have died
 *
 * @param data Pointer to main data structure
 * @param philos Pointer to array of philosophers
 * @return int 1 if any philosopher has died, 0 otherwise
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ All Philosophers Check:                         │
 * │                                                 │
 * │ 1. Iterate through all philosophers             │
 * │ 2. For each one, check if they've died          │
 * │ 3. If any one has died, return 1 immediately    │
 * │ 4. If none have died, return 0                  │
 * │                                                 │
 * │ This is called periodically by monitor thread   │
 * └─────────────────────────────────────────────────┘
 */
static int	check_all_philos(t_data *data, t_philo *philos)
{
	int	i;

	i = 0;
	while (i < data->num_philosophers)
	{
		if (check_philo_death(data, philos, i))
			return (1);
		i++;
	}
	return (0);
}

/**
 * @name check_if_all_ate
 * @brief Checks if all philosophers have eaten enough times
 *
 * @param data Pointer to main data structure
 * @param philos Pointer to array of philosophers
 * @return int 1 if all philosophers have eaten enough, 0 otherwise
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Check Algorithm:                                │
 * │                                                 │
 * │ 1. Set flag all_ate_enough = true (1)           │
 * │ 2. For each philosopher:                        │
 * │    a. Lock philosopher's meal mutex             │
 * │    b. Check if meals_eaten < must_eat_count     │
 * │    c. If any philosopher hasn't eaten enough:   │
 * │       - Set flag to false (0)                   │
 * │       - Break the loop early                    │
 * │    d. Unlock mutex and continue                 │
 * │ 3. Return the flag value                        │
 * │                                                 │
 * │ This is used to end simulation when all have    │
 * │ eaten their required number of meals            │
 * └─────────────────────────────────────────────────┘
 */
static int	check_if_all_ate(t_data *data, t_philo *philos)
{
	int	j;
	int	all_ate_enough;

	all_ate_enough = 1;
	j = 0;
	while (j < data->num_philosophers)
	{
		pthread_mutex_lock(&philos[j].meal_mutex);
		if (philos[j].meals_eaten < data->must_eat_count)
		{
			all_ate_enough = 0;
			pthread_mutex_unlock(&philos[j].meal_mutex);
			break ;
		}
		pthread_mutex_unlock(&philos[j].meal_mutex);
		j++;
	}
	return (all_ate_enough);
}

/**
 * @name wait_for_threads_ready
 * @brief Makes the monitor thread wait until all threads are ready
 *
 * @param data Pointer to main data structure
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Monitor Wait Mechanism:                         │
 * │                                                 │
 * │ 1. Lock state mutex                             │
 * │ 2. Check if all threads are ready               │
 * │ 3. If not, unlock and briefly sleep             │
 * │ 4. Reacquire lock and check again               │
 * │ 5. Once all threads are ready, continue         │
 * │ 6. Additional 1ms sleep for synchronization     │
 * │                                                 │
 * │ Similar to philosopher wait but with extra delay│
 * └─────────────────────────────────────────────────┘
 */
static void	wait_for_threads_ready(t_data *data)
{
	pthread_mutex_lock(&data->state_mutex);
	while (!data->all_threads_ready)
	{
		pthread_mutex_unlock(&data->state_mutex);
		usleep(100);
		pthread_mutex_lock(&data->state_mutex);
	}
	pthread_mutex_unlock(&data->state_mutex);
	usleep(1000);
}

/**
 * @name monitor_routine
 * @brief Monitor thread that checks for death or meal completion
 *
 * @param arg Void pointer to main data structure
 * @return void* NULL
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Monitor Thread Responsibilities:                │
 * │                                                 │
 * │ 1. Wait for all threads to be ready             │
 * │ 2. Enter monitoring loop:                       │
 * │    a. If must_eat_count is set:                 │
 * │       - Check if all philosophers have eaten    │
 * │         enough meals                            │
 * │       - If yes, stop simulation and exit        │
 * │                                                 │
 * │    b. Check if any philosopher has died         │
 * │       - If yes, exit (death already announced)  │
 * │                                                 │
 * │    c. Brief sleep to reduce CPU usage           │
 * │                                                 │
 * │ The monitor ensures simulation stops correctly  │
 * └─────────────────────────────────────────────────┘
 */
void	*monitor_routine(void *arg)
{
	t_data	*data;
	t_philo	*philos;

	data = (t_data *)arg;
	philos = data->philosophers;
	wait_for_threads_ready(data);
	while (1)
	{
		if (data->must_eat_count != -1)
		{
			if (check_if_all_ate(data, philos))
			{
				pthread_mutex_lock(&data->print_mutex);
				pthread_mutex_lock(&data->state_mutex);
				data->simulation_stop = 1;
				pthread_mutex_unlock(&data->state_mutex);
				pthread_mutex_unlock(&data->print_mutex);
				return (NULL);
			}
		}
		if (check_all_philos(data, philos))
			return (NULL);
		usleep(1000);
	}
	return (NULL);
}
