/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inits.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 15:56:27 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/08 15:59:59 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

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