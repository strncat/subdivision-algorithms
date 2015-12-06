//
//  halfedge.cpp
//  Project
//
//  Created by Fatima Broom on 8/1/15.
//  Copyright (c) 2015 nemo. All rights reserved.
//

#include "halfedge.h"

// stack overflow find boundary edges
HalfEdge* most_clockwise(HalfEdge* edge) {
    if (edge == NULL) { return NULL; }
    HalfEdge *e = edge->prev;
    while (e && e->pair) {
        e = e->pair->prev;
        if (e == edge->prev) {break;}
    }
    return e;
}

HalfEdge* most_counter_clockwise(HalfEdge* edge) {
    if (edge == NULL) { return NULL; }
    HalfEdge *e = edge;
    while (e && e->pair) {
        e = e->pair->next;
        if (e == edge) {break;}
    }
    return e;
}

void make_face(std::vector<Vertex*> &fvertices, Mesh *mesh) {
    // make new face
    Face *face = new Face();

    // push new edges
    std::vector<HalfEdge*> edges;
    for (int i = 0; i < (int)fvertices.size(); i++) { // n half edges
        edges.push_back(new HalfEdge());
    }

    // connect half edges
    int num = (int)fvertices.size();
    for (int i = 0; i < num; i++) {
        edges[i]->start = fvertices[i]; // points to the next vertex!
        edges[i]->face = face; // face
        mesh->glhalfedges.push_back(edges[i]);
        edges[i]->next = edges[(i+1)%num];
        edges[i]->prev = edges[((i+(num-1))%num)]; // useful
        if (fvertices[i]->edge == NULL) {
            fvertices[i]->edge = edges[i];
        }
    }
    face->edge = edges[0]; // face points to any half edge
    mesh->glfaces.push_back(face);

    // connect pairs after making the face // not my idea

    for (size_t i = 0; i < num; ++i) {
        HalfEdge* e = most_clockwise(fvertices[i]->edge);
        if (e && e->start->pos == edges[i]->next->start->pos
            && e->next->start->pos == edges[i]->start->pos) {
            edges[i]->pair = e;
            e->pair = edges[i];
            //printf("pairs\n");
            //e->start->pos.print();
            //edges[i]->start->pos.print();
            //printf("\n");
        }
    }
}

// FIRST TIME
void init_mesh(Mesh *mesh, std::vector<Vector3f> &draw) {
    std::vector<std::vector<int>>::iterator face;
    for (face = mesh->vertex_indices.begin(); face != mesh->vertex_indices.end(); face++) {
        std::vector<Vertex *> fvertices;
        for (int i = 0; i < face->size(); i++) {
            draw.push_back(mesh->glvertices[(*face)[i]]->pos);
            fvertices.push_back(mesh->glvertices[(*face)[i]]);
        }
        make_face(fvertices, mesh);
    }
}

void Mesh::print_mesh_size() {
    printf("faces = %ld\n", glfaces.size());
    printf("halfedges = %ld\n", glhalfedges.size());
    printf("vertices = %ld\n", glvertices.size());
}

void Mesh::delete_faces() {
    Face *f;
    while (!glfaces.empty()) {
        f = glfaces.back();
        glfaces.pop_back();
        delete f;
    }
}

void Mesh::delete_edges() {
    HalfEdge *e;
    while (!glhalfedges.empty()) {
        e = glhalfedges.back();
        glhalfedges.pop_back();
        delete e;
    }
}

void Mesh::delete_vertices() {
    Vertex *v;
    while (!glvertices.empty()) {
        v = glvertices.back();
        glvertices.pop_back();
        delete v;
    }
}

void Mesh::delete_mesh() {
    delete_faces();
    delete_edges();
    delete_vertices();
}
