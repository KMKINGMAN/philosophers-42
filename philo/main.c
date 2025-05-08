/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 17:25:34 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/08 22:04:09 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/* Philosopher actions */
int	philo_eat(t_philo *philo)
{
	t_fork	*first_fork;
	t_fork	*second_fork;

	if (philo->id % 2 == 0)
	{
		first_fork = philo->right_fork;
		second_fork = philo->left_fork;
	}
	else
	{
		first_fork = philo->left_fork;
		second_fork = philo->right_fork;
	}
	pthread_mutex_lock(&first_fork->mutex);
	print_status(philo, "has taken a fork");
	if (philo->data->num_philosophers == 1)
	{
		pthread_mutex_unlock(&first_fork->mutex);
		precise_sleep(philo->data->time_to_die);
		return (FAILURE);
	}
	pthread_mutex_lock(&second_fork->mutex);
	print_status(philo, "has taken a fork");
	pthread_mutex_lock(&philo->meal_mutex);
	philo->eating = 1;
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->meal_mutex);
	print_status(philo, "is eating");
	precise_sleep(philo->data->time_to_eat);
	pthread_mutex_lock(&philo->meal_mutex);
	philo->eating = 0;
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->meal_mutex);
	pthread_mutex_unlock(&second_fork->mutex);
	pthread_mutex_unlock(&first_fork->mutex);
	return (SUCCESS);
}

int	philo_sleep(t_philo *philo)
{
	print_status(philo, "is sleeping");
	precise_sleep(philo->data->time_to_sleep);
	return (SUCCESS);
}

/*
 * Philosopher thinks
 * This function is not strictly necessary for the simulation to work,
 * but it adds realism to the simulation by allowing philosophers to think
 * between eating and sleeping.
 * cpu hogging is happening when all philosophers are thinking at the same time
 * cpu hogging is a status when a single thread is using all the CPU time
 * and not allowing other threads to run.
 * To prevent this, we add a small delay to the think function.
 * This delay is not necessary for the simulation to work, but it prevents
 */
int	philo_think(t_philo *philo)
{
	print_status(philo, "is thinking");
	// Small delay to prevent CPU hogging
	usleep(500);
	return (SUCCESS);
}

/* Thread routines */
void	*philosopher_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;

	// Wait until all threads are ready
	pthread_mutex_lock(&philo->data->state_mutex);
	while (!philo->data->all_threads_ready)
	{
		pthread_mutex_unlock(&philo->data->state_mutex);
		usleep(100);
		pthread_mutex_lock(&philo->data->state_mutex);
	}
	pthread_mutex_unlock(&philo->data->state_mutex);
	
	// Stagger philosophers to prevent all of them from taking forks at the same time
	if (philo->id % 2 == 0)
		usleep(1000);

	// Set initial last meal time
	pthread_mutex_lock(&philo->meal_mutex);
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->meal_mutex);

	// Main philosopher cycle
	while (1)
	{
		pthread_mutex_lock(&philo->data->state_mutex);
		if (philo->data->simulation_stop)
		{
			pthread_mutex_unlock(&philo->data->state_mutex);
			break;
		}
		pthread_mutex_unlock(&philo->data->state_mutex);

		if (philo_eat(philo) == FAILURE)
			break;
		if (philo_sleep(philo) == FAILURE)
			break;
		if (philo_think(philo) == FAILURE)
			break;
	}

	return (NULL);
}

void	*monitor_routine(void *arg)
{
	t_data		*data;
	t_philo		*philos;
	int			i;
	long long	current_time;

	data = (t_data *)arg;
	philos = data->philosophers;

	// Wait until all threads start
	pthread_mutex_lock(&data->state_mutex);
	while (!data->all_threads_ready)
	{
		pthread_mutex_unlock(&data->state_mutex);
		usleep(100);
		pthread_mutex_lock(&data->state_mutex);
	}
	pthread_mutex_unlock(&data->state_mutex);
	
	usleep(1000); // Small delay to ensure philosophers have started

	while (1)
	{
		i = 0;
		// Check if all philosophers have eaten enough
		if (data->must_eat_count != -1)
		{
			int all_ate_enough = 1;
			for (int j = 0; j < data->num_philosophers; j++)
			{
				pthread_mutex_lock(&philos[j].meal_mutex);
				if (philos[j].meals_eaten < data->must_eat_count)
				{
					all_ate_enough = 0;
					pthread_mutex_unlock(&philos[j].meal_mutex);
					break;
				}
				pthread_mutex_unlock(&philos[j].meal_mutex);
			}
			
			if (all_ate_enough)
			{
				pthread_mutex_lock(&data->print_mutex);
				pthread_mutex_lock(&data->state_mutex);
				data->simulation_stop = 1;
				pthread_mutex_unlock(&data->state_mutex);
				pthread_mutex_unlock(&data->print_mutex);
				return (NULL);
			}
		}

		// Check if any philosopher died
		while (i < data->num_philosophers)
		{
			pthread_mutex_lock(&philos[i].meal_mutex);
			current_time = get_time();
			if (!philos[i].eating && 
				(current_time - philos[i].last_meal_time) >= data->time_to_die)
			{
				pthread_mutex_unlock(&philos[i].meal_mutex);
				
				// Fix deadlock by using the same locking order as print_status
				pthread_mutex_lock(&data->print_mutex);
				pthread_mutex_lock(&data->state_mutex);
				
				data->simulation_stop = 1;
				printf("%lld %d died\n", get_time() - data->start_time, philos[i].id);
				
				pthread_mutex_unlock(&data->state_mutex);
				pthread_mutex_unlock(&data->print_mutex);
				return (NULL);
			}
			pthread_mutex_unlock(&philos[i].meal_mutex);
			i++;
		}
		
		usleep(1000); // Reduce CPU usage
	}
	return (NULL);
}

/* Thread creation */
int	create_threads(t_data *data)
{
	int			i;
	pthread_t	monitor;

	data->start_time = get_time();
	
	// Initialize last_meal_time for all philosophers to start_time
	i = 0;
	while (i < data->num_philosophers)
	{
		pthread_mutex_lock(&data->philosophers[i].meal_mutex);
		data->philosophers[i].last_meal_time = data->start_time;
		pthread_mutex_unlock(&data->philosophers[i].meal_mutex);
		i++;
	}
	
	i = 0;
	while (i < data->num_philosophers)
	{
		if (pthread_create(&data->philosophers[i].thread, NULL, 
							philosopher_routine, &data->philosophers[i]) != 0)
			return (FAILURE);
		i++;
	}

	if (pthread_create(&monitor, NULL, monitor_routine, data) != 0)
		return (FAILURE);

	// All threads are ready to start
	pthread_mutex_lock(&data->state_mutex);
	data->all_threads_ready = 1;
	pthread_mutex_unlock(&data->state_mutex);
	// Wait for all philosopher threads to finish
	i = 0;
	while (i < data->num_philosophers)
	{
		if (pthread_join(data->philosophers[i].thread, NULL) != 0)
			return (FAILURE);
		i++;
	}

	// Wait for monitor thread
	if (pthread_join(monitor, NULL) != 0)
		return (FAILURE);

	return (SUCCESS);
}


/* Main function */
int	main(int argc, char **argv)
{
	t_data	data;

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