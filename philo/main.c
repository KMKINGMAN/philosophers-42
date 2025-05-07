/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 17:25:34 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/07 21:28:44 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/* Utility functions */
int	ft_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

int	ft_atoi(const char *str)
{
	int			i;
	int			sign;
	long		result;

	i = 0;
	sign = 1;
	result = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] && ft_isdigit(str[i]))
	{
		result = result * 10 + (str[i] - '0');
		if (result * sign > INT_MAX || result * sign < INT_MIN)
			return (sign == 1 ? -1 : 0);
		i++;
	}
	return (result * sign);
}

long long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	precise_sleep(long long time_in_ms)
{
	long long	start_time;

	start_time = get_time();
	while ((get_time() - start_time) < time_in_ms)
		usleep(100);
}

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

/* Initialization functions */
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
	if (data->num_philosophers <= 0 || data->time_to_die <= 0 || 
		data->time_to_eat <= 0 || data->time_to_sleep <= 0 || 
		(argc == 6 && data->must_eat_count <= 0))
		return (printf("Error: Invalid arguments\n"), FAILURE);
	data->print_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	data->state_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	return (SUCCESS);
}

int	init_forks(t_data *data)
{
	int	i;

	data->forks = malloc(sizeof(t_fork) * data->num_philosophers);
	if (!data->forks)
		return (FAILURE);
	memset(data->forks, 0, sizeof(t_fork) * data->num_philosophers);
	i = 0;
	while (i < data->num_philosophers)
	{
		data->forks[i].mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		data->forks[i].id = i;
		i++;
	}
	return (SUCCESS);
}

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
		data->philosophers[i].right_fork = &data->forks[(i + 1) % data->num_philosophers];
		data->philosophers[i].meal_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		i++;
	}
	return (SUCCESS);
}

/* Philosopher actions */
int	philo_eat(t_philo *philo)
{
	t_fork	*first_fork;
	t_fork	*second_fork;

	// To prevent deadlock, even philosophers take right fork first, odd take left fork first
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

	// If there's only one philosopher
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

/* Cleanup */
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