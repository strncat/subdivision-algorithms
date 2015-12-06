//
//  loop.cpp
//  Project
//
//  Created by Fatima B on 8/10/15.
//  Copyright (c) 2015 nemo. All rights reserved.
//

#include "loop.h"

// step (1) generate edge points
void loop_generate_edge_points(Mesh *mesh, Mesh *previous) {
    for (std::vector<HalfEdge*>::iterator i = previous->glhalfedges.begin();
         i < previous->glhalfedges.end(); i++) {
        HalfEdge **edge = &(*i);

        if ((*edge)->edgePoint) { // coming from pair
            continue;
        }

        // make a new edge point
        Vertex *edgePoint = new Vertex();

        // BOUNDARY : Just the average of that edge
        if ((*edge)->pair == NULL) {
            edgePoint->pos = ((*edge)->start->pos + (*edge)->next->start->pos) / 2.0;
            (*edge)->edgePoint = edgePoint;

        // regular case
        } else if ((*edge)->pair) {
            Vector3f edge1 = (*edge)->start->pos;
            Vector3f edge2 = (*edge)->next->start->pos;
            Vector3f edge3 = (*edge)->next->next->start->pos;
            Vector3f edge4 = (*edge)->pair->next->next->start->pos;

            edgePoint->pos = (edge3 + edge4 + edge1*3.0 + edge2*3.0) / 8.0;

            // assign to both
            (*edge)->edgePoint = edgePoint;
            (*edge)->pair->edgePoint = (*edge)->edgePoint;
        }
        
        mesh->glvertices.push_back((*edge)->edgePoint); // just another vertex
    }
}

// step (2) generate new vertices
void loop_generate_new_vertices(Mesh *mesh, Mesh *previous) {
    for (std::vector<Vertex*>::iterator i = previous->glvertices.begin();
         i < previous->glvertices.end(); i++) {

        Vertex **v = &(*i);
        Vertex *newPoint = new Vertex();
        (*v)->newPoint = newPoint;

        int valence = 0;
        HalfEdge* e = (*v)->edge;

        // loop around the vertex // flip code
        std::vector<Vertex*> neighbors;
        do {
            valence++;
            neighbors.push_back(e->edgePoint);
            // WE NEED TO GO BACKWARD
            if (e->pair == NULL) {
                HalfEdge *p = (*v)->edge->prev;
                while (p) {
                    valence++;
                    neighbors.push_back(p->edgePoint);
                    if (p->pair == NULL) { break; }
                    p = p->pair->prev;
                }
                break;
            }
            e = e->pair->next;
        } while (e != (*v)->edge);

        if (valence >= 3) {
            float beta;
            //float term = 3.0 + 2.0*cos((2.0*M_PI)/valence);
            //beta = (1.0/(8.0*valence)) * (40.0 - (term*term));
            if (valence == 3) {
                beta = 3.0 / 16.0;
            } else {
                beta = 3.0 / (valence * 8.0);
            }
            newPoint->pos = ((*v)->pos * (1.0 - valence*beta));
            for (std::vector<Vertex*>::iterator t = neighbors.begin();
                 t != neighbors.end(); t++) {
                newPoint->pos = newPoint->pos + ((*t)->pos * beta);
            }
        }
        else { // BOUNDARY VERTEX RULE 1/8 3/4 1/8
            HalfEdge *ccw_edge = most_counter_clockwise((*v)->edge);
            HalfEdge *cw_edge = most_clockwise((*v)->edge);
            newPoint->pos = ccw_edge->edgePoint->pos*(1.0/8.0) + cw_edge->edgePoint->pos*(1.0/8.0) + (*v)->pos*0.75;

            //for (std::vector<Vertex*>::iterator t = neighbors.begin();
            //     t != neighbors.end(); t++) {
            //    (*v)->newPoint->pos = (*v)->newPoint->pos + ((*t)->pos * (1.0/8.0));
            //}
            //newPoint->pos = newPoint->pos + (*v)->pos*(3.0/4.0);
        }

        (*v)->newPoint = newPoint;
        mesh->glvertices.push_back((*v)->newPoint);
    }
}

// step (3) connect
void loop_connect_new_mesh(Mesh *mesh, Mesh *previous) {
    //int index = 0;
    for (std::vector<Face*>::iterator i = previous->glfaces.begin();
         i != previous->glfaces.end(); i++) {
        HalfEdge *edge = (*i)->edge;

        // first special face
        std::vector<Vertex*> f;
        Vertex **s1 = &(edge->next->edgePoint);
        Vertex **s2 = &(edge->next->next->edgePoint);
        Vertex **s3 = &(edge->edgePoint);
        f.push_back(*s1);
        f.push_back(*s2);
        f.push_back(*s3);
        // calculate per face normal
        // begin
        Vector3f abf = (*s1)->pos - (*s2)->pos;
        Vector3f bcf = (*s2)->pos - (*s3)->pos;
        Vector3f normalf;
        cross(&normalf, &abf, &bcf);
        normalf.normalize();
        (*s1)->normal = normalf;
        (*s2)->normal = normalf;
        (*s3)->normal = normalf;
        // end
        make_face(f, mesh);

        // The other three faces
        do {
            // the new four vertices for the new face

        std::vector<Vertex*> fvertices;
        Vertex **v1 = &(edge->next->edgePoint);
        Vertex **v2 = &(edge->edgePoint);
        Vertex **v3 = &(edge->next->start->newPoint);

        // calculate per face normal
        Vector3f ab = (*v1)->pos - (*v2)->pos;
        Vector3f bc = (*v2)->pos - (*v3)->pos;
        Vector3f normal;
        cross(&normal, &ab, &bc);
        normal.normalize();
        (*v1)->normal = normal;
        (*v2)->normal = normal;
        (*v3)->normal = normal;
        // end

        fvertices.push_back(*v1);
        fvertices.push_back(*v2);
        fvertices.push_back(*v3);
        make_face(fvertices, mesh);

        //-------------------------------------
        edge = edge->next;
        } while (edge != (*i)->edge);
    }
}
