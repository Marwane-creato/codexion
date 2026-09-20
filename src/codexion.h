#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <time.h>
# include <unistd.h>

typedef struct s_config
{
	int				coder_count;
	long			burnout_time;
	long			compile_time;
	long			debug_time;
	long			refactor_time;
	long			required_compiles;
	long			cooldown_time;
	int				use_edf;
}	t_config;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	int				in_use;
	long			available_at;
}	t_dongle;

typedef struct s_request
{
	int				coder_id;
	long			order;
	long			deadline;
}	t_request;

typedef struct s_simulation	t_simulation;
typedef struct s_coder
{
	int				coder_id;
	int				compile_count;
	int				finished;
	long			last_compile_start;
	pthread_t		thread;
	t_simulation	*simulation;
}	t_coder;

struct s_simulation
{
	t_config		config;
	t_dongle		*dongles;
	t_coder			*coders;
	t_request		*requests;
	int				request_count;
	long			next_request_order;
	long			start_time;
	int				stopped;
	int				completed_coders;
	pthread_mutex_t	state_mutex;
	pthread_mutex_t	queue_mutex;
	pthread_mutex_t	print_mutex;
	pthread_cond_t	resource_changed;
	pthread_t		monitor_thread;
};

long	current_time_ms(void);
int		parse_arguments(int argc, char **argv, t_config *config);
int		request_push(t_simulation *sim, t_request request);
void	request_remove(t_simulation *sim, int coder_id);
int		request_take_available(t_simulation *sim, t_coder *coder);
int		init_simulation(t_simulation *sim, t_config *config);
void	destroy_simulation(t_simulation *sim);
void	print_state(t_simulation *sim, int coder_id, const char *state);
int		simulation_stopped(t_simulation *sim);
void	wait_ms(t_simulation *sim, long milliseconds);
void	*coder_routine(void *argument);
void	*monitor_routine(void *argument);
int		reserve_pair(t_coder *coder, int reserve);
void	release_pair(t_coder *coder);
void	wait_for_change(t_simulation *sim);
void	add_request(t_coder *coder, t_request *request);
#endif
