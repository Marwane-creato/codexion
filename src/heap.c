#include "codexion.h"

static int	comes_first(t_simulation *sim, t_request first, t_request second)
{
	if (!sim->config.use_edf)
		return (first.order < second.order);
	if (first.deadline != second.deadline)
		return (first.deadline < second.deadline);
	return (first.order < second.order);
}

int	request_push(t_simulation *sim, t_request request)
{
	int				index;
	int				parent;
	t_request		temporary;

	index = sim->request_count;
	sim->request_count++;
	sim->requests[index] = request;
	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!comes_first(sim, sim->requests[index], sim->requests[parent]))
			break ;
		temporary = sim->requests[index];
		sim->requests[index] = sim->requests[parent];
		sim->requests[parent] = temporary;
		index = parent;
	}
	return (1);
}

static void	move_down(t_simulation *sim, int index)
{
	int				left;
	int				right;
	int				best;
	t_request		temporary;

	while (1)
	{
		left = index * 2 + 1;
		right = left + 1;
		best = index;
		if (left < sim->request_count && comes_first(sim,
				sim->requests[left], sim->requests[best]))
			best = left;
		if (right < sim->request_count && comes_first(sim,
				sim->requests[right], sim->requests[best]))
			best = right;
		if (best == index)
			break ;
		temporary = sim->requests[index];
		sim->requests[index] = sim->requests[best];
		sim->requests[best] = temporary;
		index = best;
	}
}

void	request_remove(t_simulation *sim, int coder_id)
{
	int				index;
	t_request		temporary;

	index = 0;
	while (index < sim->request_count
		&& sim->requests[index].coder_id != coder_id)
		index++;
	if (index == sim->request_count)
		return ;
	sim->request_count--;
	sim->requests[index] = sim->requests[sim->request_count];
	move_down(sim, index);
	while (index > 0 && comes_first(sim, sim->requests[index],
			sim->requests[(index - 1) / 2]))
	{
		temporary = sim->requests[index];
		sim->requests[index] = sim->requests[(index - 1) / 2];
		sim->requests[(index - 1) / 2] = temporary;
		index = (index - 1) / 2;
	}
}

int	request_take_available(t_simulation *sim, t_coder *coder)
{
	int	index;
	int	best;

	best = -1;
	index = 0;
	while (index < sim->request_count)
	{
		if (reserve_pair(&sim->coders[sim->requests[index].coder_id - 1], 0)
			&& (best == -1 || comes_first(sim, sim->requests[index],
					sim->requests[best])))
			best = index;
		index++;
	}
	if (best == -1 || sim->requests[best].coder_id != coder->coder_id)
		return (0);
	return (reserve_pair(coder, 1));
}
