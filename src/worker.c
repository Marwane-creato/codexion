#include "codexion.h"

static int	end_request(t_coder *coder, int reserved);

static int	get_dongles(t_coder *coder)
{
	t_simulation	*sim;
	t_request		request;
	int				reserved;

	sim = coder->simulation;
	if (sim->config.coder_count == 1)
		return (0);
	pthread_mutex_lock(&sim->queue_mutex);
	add_request(coder, &request);
	request_push(sim, request);
	reserved = 0;
	while (!simulation_stopped(sim) && !reserved)
	{
		reserved = request_take_available(sim, coder);
		if (!reserved)
			wait_for_change(sim);
	}
	if (!end_request(coder, reserved))
		return (0);
	print_state(sim, coder->coder_id, "has taken a dongle");
	print_state(sim, coder->coder_id, "has taken a dongle");
	return (1);
}

static int	end_request(t_coder *coder, int reserved)
{
	t_simulation	*sim;

	sim = coder->simulation;
	request_remove(sim, coder->coder_id);
	pthread_cond_broadcast(&sim->resource_changed);
	pthread_mutex_unlock(&sim->queue_mutex);
	if (simulation_stopped(sim))
	{
		if (reserved)
			release_pair(coder);
		return (0);
	}
	return (1);
}

static void	count_compile(t_coder *coder)
{
	t_simulation	*sim;

	sim = coder->simulation;
	pthread_mutex_lock(&sim->state_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&sim->state_mutex);
}

static void	finish_coder(t_coder *coder)
{
	t_simulation	*sim;

	sim = coder->simulation;
	pthread_mutex_lock(&sim->state_mutex);
	coder->finished = 1;
	sim->completed_coders++;
	if (sim->completed_coders == sim->config.coder_count)
	{
		sim->stopped = 1;
		pthread_cond_broadcast(&sim->resource_changed);
	}
	pthread_mutex_unlock(&sim->state_mutex);
}

static int	compile_once(t_coder *coder)

{
	t_simulation	*sim;

	sim = coder->simulation;
	if (!get_dongles(coder))
		return (0);
	pthread_mutex_lock(&sim->state_mutex);
	coder->last_compile_start = current_time_ms();
	pthread_mutex_unlock(&sim->state_mutex);
	print_state(sim, coder->coder_id, "is compiling");
	wait_ms(sim, sim->config.compile_time);
	release_pair(coder);
	count_compile(coder);
	return (!simulation_stopped(sim));
}

void	*coder_routine(void *argument)
{
	t_coder			*coder;
	t_simulation	*sim;

	coder = argument;
	sim = coder->simulation;
	while (!simulation_stopped(sim)
		&& coder->compile_count < sim->config.required_compiles)
	{
		if (!compile_once(coder))
			break ;
		print_state(sim, coder->coder_id, "is debugging");
		wait_ms(sim, sim->config.debug_time);
		if (simulation_stopped(sim))
			break ;
		print_state(sim, coder->coder_id, "is refactoring");
		wait_ms(sim, sim->config.refactor_time);
		if (simulation_stopped(sim))
			break ;
		if (coder->compile_count == sim->config.required_compiles)
			finish_coder(coder);
	}
	return (NULL);
}
