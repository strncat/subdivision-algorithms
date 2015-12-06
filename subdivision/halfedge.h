//
//  halfedge.h
//  Project
//
//  Created by Fatima Broom on 8/1/15.
//  Copyright (c) 2015 nemo. All rights reserved.
//

#ifndef HALFEDGE_H
#define HALFEDGE_H

#include <stdio.h>
#include <unordered_map>
#include "util.h"

class Vertex;
class Face;
class HalfEdge;

struct Vertex {
    Vector3f pos;
    Vector3f normal;
    Vector3f texture;
    HalfEdge *edge;
    Vertex() {edge = NULL;}
    Vertex(Vector3f p) {edge = NULL; pos = p;}
    Vertex(Vertex const &v) {this->pos = v.pos; this->edge = v.edge;}
    Vertex *newPoint; // future newPoint
};

struct Face {
    // Each face references one of the HalfEdges the belongs to it.
    HalfEdge *edge;
    Vertex *facePoint;
    Face() {
        edge = NULL;
        facePoint = NULL;
    }
};

struct HalfEdge {
    Vertex *start; // the Vertex it points to
    HalfEdge *next; // the next HalfEdge inside the face
    HalfEdge *prev; // the prev HalfEdge
    HalfEdge *pair; // the pair HalfEdge
    Face *face; // the face it belongs to
    Vertex* edgePoint; // its future edge point
    //bool boundary;
    HalfEdge() {
        start = NULL;
        next = NULL;
        prev = NULL;
        pair = NULL;
        face = NULL;
        edgePoint = NULL;
    }
};

class Mesh {
public:
    std::vector<HalfEdge*> glhalfedges;
    std::vector<Vertex*> glvertices;
    std::vector<Face*> glfaces;

    // methods
    void delete_faces();
    void delete_edges();
    void delete_vertices();
    void delete_mesh();
    void print_mesh_size();

    // LOAD DATA RELATED METHODS
    Mesh() {hasTextures = false; quad = false;}
    std::vector<std::vector<int>> vertex_indices;
    bool hasTextures;
    bool quad;
    bool load_file(const std::string& filename);
    bool check_index(int index, size_t size);
    bool load_object(const std::string& filename);
};

// USEFUL GENERIC METHODS
void init_mesh(Mesh *mesh, std::vector<Vector3f> &draw);
void make_face(std::vector<Vertex*> &fvertices, Mesh *mesh);
HalfEdge* most_clockwise(HalfEdge* edge);
HalfEdge* most_counter_clockwise(HalfEdge* edge);

#endif
