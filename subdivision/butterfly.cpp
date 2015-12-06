//
//  butterfly.cpp
//  A4
//
//  Created by Fatima B on 8/12/15.
//  Copyright (c) 2015 nemo. All rights reserved.
//

#include "butterfly.h"

// step (1) generate edge points
void butterfly_generate_edge_points(Mesh *mesh, Mesh *previous) {
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
            edgePoint->pos = ((*edge)->start->pos + (*edge)->next->start->pos) * 0.5;
            (*edge)->edgePoint = edgePoint;
            (*edge)->edgePoint->pos.print();

            // not bounday
        } else if ((*edge)->pair) {

            // weights multiple cases her
            // http://www.cs.cmu.edu/afs/cs/academic/class/15462-s12/www/lec_slides/lec07.pdf

            Vector3f test(1.5, 2.25, 0.0);
            Vector3f test2(2.25, 2.25, 0.0);

            if ((*edge)->start->pos == test2) {
                printf("yes\n");
            }

            HalfEdge *edge1 = (*edge);
            HalfEdge *edge2 = (*edge)->next;

            HalfEdge *edge3 = (*edge)->next->next;
            HalfEdge *edge4 = (*edge)->pair->next->next;

            Vector3f a1 = edge1->start->pos;
            Vector3f a2 = edge2->start->pos;

            Vector3f b1 = edge3->start->pos;
            Vector3f b2 = edge4->start->pos;

            Vector3f c1, c2, c3, c4;

            // case 1 regular vertex
            if (edge2->pair && edge3->pair && edge1->pair // upper two triangles
                    && edge1->pair->next->next->pair && edge1->pair->next->pair) { // lower two triangles

                c2 = edge2->pair->next->next->start->pos;
                c1 = edge3->pair->next->next->start->pos;

                c3 = edge1->pair->next->next->pair->next->next->start->pos;
                c4 = edge1->pair->next->pair->next->next->start->pos;

                edgePoint->pos = (a1 + a2)*(1.0/2.0) + (b1 + b2)*(1.0/8.0) - (c1 + c2 + c3 + c4)*(1.0/16.0);
            }

            //
            else {
                edgePoint->pos = (a1*9.0 + a2*9.0 - b1 - b2) * (1.0/16.0);
            }

            // assign to both
            (*edge)->edgePoint = edgePoint;
            (*edge)->pair->edgePoint = (*edge)->edgePoint;
        }

        mesh->glvertices.push_back((*edge)->edgePoint); // just another vertex
    }
}

// step (2) connect
void butterfly_connect_new_mesh(Mesh *mesh, Mesh *previous) {
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
        Vertex **v3 = &(edge->next->start);

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

        edge = edge->next;
        } while (edge != (*i)->edge);
    }
}
