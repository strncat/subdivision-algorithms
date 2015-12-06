//
//  butterfly.h
//  A4
//
//  Created by Fatima B on 8/12/15.
//  Copyright (c) 2015 nemo. All rights reserved.
//

#ifndef BUTTERFLY
#define BUTTERFLY

#include <stdio.h>

#include "halfedge.h"

// step (1) generate edge points
void butterfly_generate_edge_points(Mesh *mesh, Mesh *previous);

// step (2) connect
void butterfly_connect_new_mesh(Mesh *mesh, Mesh *previous);


#endif
