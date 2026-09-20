#include "codexion.h"

long	current_time_ms(void)
{
	struct timeval	time;

	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000L + time.tv_usec / 1000);
}

int	simulation_stopped(t_simulation *sim)
{
	int	stopped;

	pthread_mutex_lock(&sim->state_mutex);
	stopped = sim->stopped;
	pthread_mutex_unlock(&sim->state_mutex);
	return (stopped);
}

void	wait_ms(t_simulation *sim, long milliseconds)
{
	long	end_time;

	end_time = current_time_ms() + milliseconds;
	while (!simulation_stopped(sim) && current_time_ms() < end_time)
		usleep(500);
}

void	print_state(t_simulation *sim, int coder_id, const char *state)
{
	pthread_mutex_lock(&sim->print_mutex);
	pthread_mutex_lock(&sim->state_mutex);
	if (!sim->stopped || strcmp(state, "burned out") == 0)
		printf("%ld %d %s\n", current_time_ms() - sim->start_time,
			coder_id, state);
	pthread_mutex_unlock(&sim->state_mutex);
	pthread_mutex_unlock(&sim->print_mutex);
}
