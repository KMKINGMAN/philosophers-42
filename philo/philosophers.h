/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosophers.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 17:27:46 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/21 20:26:25 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILOSOPHERS_H
# define PHILOSOPHERS_H
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <limits.h>
# include <sys/time.h>

typedef enum e_exit_status
{
	SUCCESS = 0,
	FAILURE = 1
} t_exit_status;

/* Commented out because using enum instead
# define SUCCESS 0
# define FAILURE 1
*/

typedef struct s_data	t_data;

typedef struct s_fork
{
	pthread_mutex_t	mutex;
	int				id;
}	t_fork;

typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	int				eating;
	long long		last_meal_time;
	pthread_t		thread;
	t_fork			*left_fork;
	t_fork			*right_fork;
	pthread_mutex_t	meal_mutex;
	t_data			*data;
}	t_philo;

typedef struct s_data
{
	int				num_philosophers;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				must_eat_count;
	int				all_threads_ready;
	int				simulation_stop;
	long long		start_time;
	t_philo			*philosophers;
	t_fork			*forks;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	state_mutex;
}	t_data;

/* Init functions */
int			init_data(t_data *data, int argc, char **argv);
int			init_philosophers(t_data *data);
int			init_forks(t_data *data);

/* Thread and routine functions */
int			create_threads(t_data *data);
void		*philosopher_routine(void *arg);
void		*monitor_routine(void *arg);

/* Action functions */
int			philo_eat(t_philo *philo);
int			philo_sleep(t_philo *philo);
int			philo_think(t_philo *philo);

/* Utils functions */
int			ft_atoi(const char *str);
int			ft_isdigit(int c);
long long	get_time(void);
void		precise_sleep(long long time_in_ms);
void		print_status(t_philo *philo, char *status);

/* Cleanup functions */
void		free_data(t_data *data);

#endif