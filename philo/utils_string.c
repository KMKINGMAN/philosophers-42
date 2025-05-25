/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_string.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkurkar <mkurkar@student.42amman.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 15:53:29 by mkurkar           #+#    #+#             */
/*   Updated: 2025/05/25 09:53:01 by mkurkar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/**
 * @name ft_isdigit
 * @brief Checks if a character is a digit (0-9)
 *
 * @param c Character to check
 * @return int 1 if the character is a digit, 0 otherwise
 *
 * Example:
 * ┌─────────────────────────────┐
 * │ ft_isdigit('5') → 1 (true)  │
 * │ ft_isdigit('a') → 0 (false) │
 * └─────────────────────────────┘
 */
int	ft_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

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
	int		i;
	int		sign;
	long	result;

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
		{
			if (sign == 1)
				return (-1);
			else
				return (0);
		}
		i++;
	}
	return (result * sign);
}
