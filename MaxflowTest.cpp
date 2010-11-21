#include <stdio.h>
#include "graph.h"

int main()
{
  typedef Graph GraphType;
  GraphType *g = new GraphType;

  g -> add_node();
  g -> add_node();

  int zero = 0;
  int one = 1;
  g -> add_tweights( &zero,   /* capacities */  1, 5 );
  g -> add_tweights( &one,   /* capacities */  2, 6 );
  g -> add_edge( &zero, &one,    /* capacities */  3, 4 );

  int flow = g -> maxflow();

  printf("Flow = %d\n", flow);
  printf("Minimum cut:\n");
  if (g->what_segment(&zero) == GraphType::SOURCE)
    printf("node0 is in the SOURCE set\n");
  else
    printf("node0 is in the SINK set\n");
  if (g->what_segment(&one) == GraphType::SOURCE)
    printf("node1 is in the SOURCE set\n");
  else
    printf("node1 is in the SINK set\n");

  delete g;

  return 0;
}