#include "codexion.h"

static void	join_coders(t_simulation *sim)
{
	int	index;

	index = 0;
	while (index < sim->config.coder_count)
	{
		pthread_join(sim->coders[index].thread, NULL);
		index++;
	}
}

static int	print_error(const char *message)
{
	fprintf(stderr, "%s\n", message);
	return (1);
}

int	main(int argc, char **argv)
{
	t_config		config;
	t_simulation	sim;
	int				index;

	if (!parse_arguments(argc, argv, &config))
		return (print_error("Invalid arguments"));
	if (!init_simulation(&sim, &config))
		return (print_error("Allocation error"));
	sim.start_time = current_time_ms();
	index = 0;
	while (index < config.coder_count)
	{
		sim.coders[index].last_compile_start = sim.start_time;
		index++;
	}
	pthread_create(&sim.monitor_thread, NULL, monitor_routine, &sim);
	index = 0;
	while (index < config.coder_count)
	{
		pthread_create(&sim.coders[index].thread, NULL, coder_routine,
			&sim.coders[index]);
		index++;
	}
	join_coders(&sim);
	pthread_join(sim.monitor_thread, NULL);
	destroy_simulation(&sim);
	return (0);
}
