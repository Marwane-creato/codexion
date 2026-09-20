#include "codexion.h"

static void	free_arrays(t_simulation *sim)
{
	free(sim->dongles);
	free(sim->coders);
	free(sim->requests);
}

static void	init_coders(t_simulation *sim)
{
	int	index;

	index = 0;
	while (index < sim->config.coder_count)
	{
		pthread_mutex_init(&sim->dongles[index].mutex, NULL);
		sim->coders[index].coder_id = index + 1;
		sim->coders[index].simulation = sim;
		index++;
	}
}

int	init_simulation(t_simulation *sim, t_config *config)
{
	memset(sim, 0, sizeof(*sim));
	sim->config = *config;
	sim->dongles = malloc(sizeof(t_dongle) * config->coder_count);
	sim->coders = malloc(sizeof(t_coder) * config->coder_count);
	sim->requests = malloc(sizeof(t_request) * config->coder_count);
	if (!sim->dongles || !sim->coders || !sim->requests)
	{
		free_arrays(sim);
		return (0);
	}
	memset(sim->dongles, 0, sizeof(t_dongle) * config->coder_count);
	memset(sim->coders, 0, sizeof(t_coder) * config->coder_count);
	memset(sim->requests, 0, sizeof(t_request) * config->coder_count);
	pthread_mutex_init(&sim->state_mutex, NULL);
	pthread_mutex_init(&sim->queue_mutex, NULL);
	pthread_mutex_init(&sim->print_mutex, NULL);
	pthread_cond_init(&sim->resource_changed, NULL);
	init_coders(sim);
	return (1);
}

void	destroy_simulation(t_simulation *sim)
{
	int	index;

	index = 0;
	while (index < sim->config.coder_count)
	{
		pthread_mutex_destroy(&sim->dongles[index].mutex);
		index++;
	}
	pthread_cond_destroy(&sim->resource_changed);
	pthread_mutex_destroy(&sim->print_mutex);
	pthread_mutex_destroy(&sim->queue_mutex);
	pthread_mutex_destroy(&sim->state_mutex);
	free_arrays(sim);
}
