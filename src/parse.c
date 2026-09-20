#include "codexion.h"

static int	read_positive_number(const char *text, long *value)
{
	long	number;

	if (*text == '\0')
		return (0);
	number = 0;
	while (*text)
	{
		if (*text < '0' || *text > '9' || number > 214748364)
			return (0);
		number = number * 10 + (*text - '0');
		text++;
	}
	if (number > 2147483647)
		return (0);
	*value = number;
	return (1);
}

static void	set_config(t_config *config, long *values, char *scheduler)
{
	config->coder_count = values[0];
	config->burnout_time = values[1];
	config->compile_time = values[2];
	config->debug_time = values[3];
	config->refactor_time = values[4];
	config->required_compiles = values[5];
	config->cooldown_time = values[6];
	config->use_edf = (strcmp(scheduler, "edf") == 0);
}

int	parse_arguments(int argc, char **argv, t_config *config)
{
	long	values[7];
	int		index;

	if (argc != 9)
		return (0);
	index = 0;
	while (index < 7)
	{
		if (!read_positive_number(argv[index + 1], &values[index]))
			return (0);
		index++;
	}
	if (values[0] < 1 || values[1] < 1 || values[2] < 1
		|| values[3] < 1 || values[4] < 1 || values[5] < 1)
		return (0);
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (0);
	set_config(config, values, argv[8]);
	return (1);
}
