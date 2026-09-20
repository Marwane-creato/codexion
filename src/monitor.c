#include "codexion.h"

static int	find_burned_coder(t_simulation *sim)
{
	int	index;

	index = 0;
	while (index < sim->config.coder_count)
	{
		if (!sim->coders[index].finished
			&& current_time_ms() >= sim->coders[index].last_compile_start
			+ sim->config.burnout_time)
			return (sim->coders[index].coder_id);
		index++;
	}
	return (0);
}

void	*monitor_routine(void *argument)
{
	t_simulation	*sim;
	int				burned_coder;

	sim = argument;
	while (!simulation_stopped(sim))
	{
		pthread_mutex_lock(&sim->state_mutex);
		burned_coder = find_burned_coder(sim);
		if (burned_coder)
			sim->stopped = 1;
		pthread_mutex_unlock(&sim->state_mutex);
		if (burned_coder)
		{
			print_state(sim, burned_coder, "burned out");
			pthread_cond_broadcast(&sim->resource_changed);
			break ;
		}
		usleep(500);
	}
	return (NULL);
}
