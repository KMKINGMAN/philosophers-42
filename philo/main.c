/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 17:25:34 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/22 14:19:03 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/* Philosopher actions */
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
 * @name philo_sleep
 * @brief Handles the sleeping action of a philosopher
 * 
 * @param philo Pointer to philosopher structure
 * @return int SUCCESS always
 * 
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Sleeping Process:                               │
 * │                                                 │
 * │ 1. Print sleeping status                        │
 * │ 2. Sleep for time_to_sleep milliseconds         │
 * │ 3. Return success                               │
 * │                                                 │
 * │ This simulates the philosopher resting          │
 * └─────────────────────────────────────────────────┘
 */
int	philo_sleep(t_philo *philo)
{
	print_status(philo, "is sleeping");
	precise_sleep(philo->data->time_to_sleep);
	return (SUCCESS);
}

/**
 * @name philo_think
 * @brief Handles the thinking action of a philosopher
 * 
 * @param philo Pointer to philosopher structure
 * @return int SUCCESS always
 * 
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Thinking Process:                               │
 * │                                                 │
 * │ 1. Print thinking status                        │
 * │ 2. Brief pause (usleep 500 microseconds)        │
 * │ 3. Return success                               │
 * │                                                 │
 * │ This simulates the philosopher contemplating    │
 * └─────────────────────────────────────────────────┘
 */
int	philo_think(t_philo *philo)
{
	print_status(philo, "is thinking");
	usleep(500);
	return (SUCCESS);
}

/* Thread routines */
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
static void	wait_for_all_threads(t_philo *philo)
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
			break;
		}
		pthread_mutex_unlock(&philo->data->state_mutex);

		if (philo_eat(philo) == FAILURE \
			|| philo_sleep(philo) == FAILURE \
			|| philo_think(philo) == FAILURE
		)
			break;
	}
	return (NULL);
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
	if (!philos[i].eating && 
		(current_time - philos[i].last_meal_time) >= data->time_to_die)
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
	t_data		*data;
	t_philo		*philos;

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