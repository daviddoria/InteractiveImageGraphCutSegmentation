/* graph.cpp */
/*
 *
 * Licence.

Copyright UCL Business PLC

This program is available under dual licence:

1) Under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any
later version.
Note that any program that incorporates the code under this licence must, under the terms of the GNU GPL, be released under a licence compatible with the GP
L. GNU GPL does not permit incorporating this program into proprietary programs. If you wish to do this, please see the alternative licence available below.
GNU General Public License can be found at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html

2) Proprietary Licence from UCL Business PLC.
To enable programers to include the MaxFlow software in a proprietary system (which is not allowed by the GNU GPL), this licence gives you the right to inco
rporate the software in your program and distribute under any licence of your choosing. The full terms of the licence and applicable fee, are available from
 the Licensors at: http://www.uclb-elicensing.com/optimisation_software/maxflow_computervision.html
*/

#include <stdio.h>
#include "graph.h"

Graph::Graph(void (*err_function)(const char *))
{
	error_function = err_function;
	node_block = new Block<node>(NODE_BLOCK_SIZE, error_function);
	arc_block  = new Block<arc>(NODE_BLOCK_SIZE, error_function);
	flow = 0;
}

Graph::~Graph()
{
	delete node_block;
	delete arc_block;
}

Graph::node_id Graph::add_node()
{
	node *i = node_block -> New();

	i -> first = NULL;
	i -> tr_cap = 0;

	return (node_id) i;
}

void Graph::add_edge(node_id from, node_id to, captype cap, captype rev_cap)
{
	arc *a, *a_rev;

	a = arc_block -> New(2);
	a_rev = a + 1;

	a -> sister = a_rev;
	a_rev -> sister = a;
	a -> next = ((node*)from) -> first;
	((node*)from) -> first = a;
	a_rev -> next = ((node*)to) -> first;
	((node*)to) -> first = a_rev;
	a -> head = (node*)to;
	a_rev -> head = (node*)from;
	a -> r_cap = cap;
	a_rev -> r_cap = rev_cap;
}

void Graph::set_tweights(node_id i, captype cap_source, captype cap_sink)
{
	flow += (cap_source < cap_sink) ? cap_source : cap_sink;
	((node*)i) -> tr_cap = cap_source - cap_sink;
}

void Graph::add_tweights(node_id i, captype cap_source, captype cap_sink)
{
	register captype delta = ((node*)i) -> tr_cap;
	if (delta > 0) cap_source += delta;
	else           cap_sink   -= delta;
	flow += (cap_source < cap_sink) ? cap_source : cap_sink;
	((node*)i) -> tr_cap = cap_source - cap_sink;
}
