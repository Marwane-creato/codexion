#include "codexion.h"

static void	get_pair(t_coder *coder, int *left, int *right)
{
	*left = coder->coder_id - 1;
	*right = coder->coder_id % coder->simulation->config.coder_count;
	if (*left > *right)
	{
		*left = 0;
		*right = coder->simulation->config.coder_count - 1;
	}
}

int	reserve_pair(t_coder *coder, int reserve)
{
	t_simulation	*sim;
	int				left;
	int				right;
	long			now;
	int				available;

	sim = coder->simulation;
	get_pair(coder, &left, &right);
	now = current_time_ms();
	pthread_mutex_lock(&sim->dongles[left].mutex);
	pthread_mutex_lock(&sim->dongles[right].mutex);
	available = !sim->dongles[left].in_use && !sim->dongles[right].in_use;
	available = available && now >= sim->dongles[left].available_at;
	available = available && now >= sim->dongles[right].available_at;
	if (available && reserve)
	{
		sim->dongles[left].in_use = 1;
		sim->dongles[right].in_use = 1;
	}
	pthread_mutex_unlock(&sim->dongles[right].mutex);
	pthread_mutex_unlock(&sim->dongles[left].mutex);
	return (available);
}

void	release_pair(t_coder *coder)
{
	t_simulation	*sim;
	int				left;
	int				right;
	long			ready;

	sim = coder->simulation;
	get_pair(coder, &left, &right);
	pthread_mutex_lock(&sim->dongles[left].mutex);
	pthread_mutex_lock(&sim->dongles[right].mutex);
	ready = current_time_ms() + sim->config.cooldown_time;
	sim->dongles[left].in_use = 0;
	sim->dongles[right].in_use = 0;
	sim->dongles[left].available_at = ready;
	sim->dongles[right].available_at = ready;
	pthread_mutex_unlock(&sim->dongles[right].mutex);
	pthread_mutex_unlock(&sim->dongles[left].mutex);
	pthread_cond_broadcast(&sim->resource_changed);
}

void	wait_for_change(t_simulation *sim)
{
	struct timespec	timeout;

	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_nsec += 1000000L;
	if (timeout.tv_nsec >= 1000000000L)
	{
		timeout.tv_sec++;
		timeout.tv_nsec -= 1000000000L;
	}
	pthread_cond_timedwait(&sim->resource_changed, &sim->queue_mutex,
		&timeout);
}

void	add_request(t_coder *coder, t_request *request)
{
	t_simulation	*sim;

	sim = coder->simulation;
	request->coder_id = coder->coder_id;
	request->order = sim->next_request_order;
	sim->next_request_order++;
	request->deadline = coder->last_compile_start + sim->config.burnout_time;
}
