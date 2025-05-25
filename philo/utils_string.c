/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_string.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 15:53:29 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/25 16:00:04 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/**
 * @name ft_atoi
 * @brief Converts a string to an integer
 *
 * @param str String to convert
 * @return int The converted integer value
 *
 * Example:
 * ┌───────────────────────────────────────────┐
 * │ ft_atoi("42") → 42                        │
 * │ ft_atoi("-42") → -42                      │
 * │ ft_atoi("  +42") → 42                     │
 * │ ft_atoi("2147483648") → -1 (overflow)     │
 * └───────────────────────────────────────────┘
 */
int	ft_atoi(const char *str)
{
	int	i;
	int	sign;
	int	res;

	i = 0;
	sign = 1;
	res = 0;
	while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\v'
		|| str[i] == '\f' || str[i] == '\r')
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		res = res * 10 + str[i] - '0';
		i++;
	}
	return (res * sign);
}
