//
//  catmullclark.h
//  Project
//
//  Created by Fatima Broom on 8/3/15.
//  Copyright (c) 2015 nemo. All rights reserved.
//

#ifndef CATMULLCLARK
#define CATMULLCLARK

#include "halfedge.h"

// step (1) generate face points
void generate_face_points(Mesh *mesh, Mesh *previous);

// step (2) generate edge points
void generate_edge_points(Mesh *mesh, Mesh *previous);

// step (3) generate new vertices
void generate_new_vertices(Mesh *mesh, Mesh *previous);

// step (4) connect
void connect_new_mesh(Mesh *mesh, Mesh *previous);
#endif
