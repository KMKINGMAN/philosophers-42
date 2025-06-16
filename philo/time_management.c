/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_management.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 15:54:39 by mkurkar           #+#    #+#             */
/*   Updated: 2025/06/16 10:31:28 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/**
 * @name get_time
 * @brief Gets the current time in milliseconds
 *
 * @return long long Current time in milliseconds since epoch
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Time Calculation:                               │
 * │                                                 │
 * │ seconds × 1000 + microseconds ÷ 1000 = millisec │
 * │                                                 │
 * │ 1 second = 1000 milliseconds                    │
 * │ 1000 microseconds = 1 millisecond               │
 * └─────────────────────────────────────────────────┘
 */
long long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/**
 * @name precise_sleep
 * @brief Sleeps for a specified amount of time with high precision
 *
 * @param time_in_ms Time to sleep in milliseconds
 *
 * Example:
 * ┌─────────────────────────────────────────────────┐
 * │ Precision Sleep Implementation:                 │
 * │                                                 │
 * │ 1. Record start time                            │
 * │ 2. Loop until current_time - start ≥ target     │
 * │ 3. Small usleep(100) to avoid CPU hogging       │
 * │                                                 │
 * │ More precise than standard sleep functions      │
 * └─────────────────────────────────────────────────┘
 */
void	precise_sleep(long long time_in_ms)
{
	long long	start_time;

	start_time = get_time();
	while ((get_time() - start_time) < time_in_ms)
		usleep(100);
}

/**
 * @name interruptible_sleep
 * @brief Sleeps for a specified time but can be interrupted by simulation stop
 *
 * @param philo Pointer to philosopher structure
 * @param time_in_ms Time to sleep in milliseconds
 * @return int SUCCESS if completed normally, FAILURE if interrupted
 */
int	interruptible_sleep(t_philo *philo, long long time_in_ms)
{
	long long	start_time;

	start_time = get_time();
	while ((get_time() - start_time) < time_in_ms)
	{
		if (check_simulation_stop(philo))
			return (FAILURE);
		usleep(100);
	}
	return (SUCCESS);
}
