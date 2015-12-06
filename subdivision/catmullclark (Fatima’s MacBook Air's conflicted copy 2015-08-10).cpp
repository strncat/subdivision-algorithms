//
//  catmullclark.cpp
//  A4
//
//  Created by Fatima Broom on 8/2/15.
//  Copyright (c) 2015 nemo. All rights reserved.
//

#include "catmullclark.h"

// (1)
void generate_face_points(Mesh *mesh, Mesh *previous) {
    for (std::vector<Face*>::iterator f = previous->glfaces.begin(); f != previous->glfaces.end(); f++) {

        // create new vertex for the face point
        Vertex* facePoint = new Vertex();

        // iterate through the face // flip code
        HalfEdge *e = (*f)->edge;
        int num = 0;
        do {
            num++;
            facePoint->pos = facePoint->pos + e->start->pos; // add the vertex position
            e = e->next;
        } while (e != (*f)->edge);
        facePoint->pos = facePoint->pos / num;

        (*f)->facePoint = facePoint;
        mesh->glvertices.push_back((*f)->facePoint); // just another vertex
    }
}

// (2)
void generate_edge_points(Mesh *mesh, Mesh *previous) {
    for (std::vector<HalfEdge*>::iterator i = previous->glhalfedges.begin(); i < previous->glhalfedges.end(); i++) {
        HalfEdge **edge = &(*i);

        if ((*edge)->edgePoint) { // coming from pair
            continue;
        }

        Vertex *edgePoint = new Vertex(); // make a new edge point

        if ((*edge)->pair == NULL) { // BOUNDARY : Just the average of that edge
            (*edgePoint).pos = ((*edge)->start->pos + (*edge)->next->start->pos) / 2.0;
            (*edge)->edgePoint = edgePoint;

        } else if ((*edge)->pair) { // not boundary edge
            Vector3f edge1 = (*edge)->start->pos;
            Vector3f edge2 = (*edge)->next->start->pos;
            Vector3f face1 = (*edge)->face->facePoint->pos;
            Vector3f face2 = (*edge)->pair->face->facePoint->pos;

            edgePoint->pos = (face1 + face2 + edge1 + edge2) / 4.0;

            // assign to both
            (*edge)->edgePoint = edgePoint;
            (*edge)->pair->edgePoint = (*edge)->edgePoint;
        }
        mesh->glvertices.push_back((*edge)->edgePoint); // just another vertex
    }
}

void generate_new_vertices(Mesh *mesh, Mesh *previous) { // original vertices in the mesh

    for (int i = 0; i < previous->glvertices.size(); i++) {
        Vertex **v = &(previous->glvertices[i]);
        Vertex *face_average;
        Vertex *edge_average;

        auto sz = avg_adj_facepts(*v, &face_average);
        auto sz2 = avg_adj_edge_midpts(*v, &edge_average);

        if (sz == sz2) {
            //printf("not boundary\n");

            float m1 = 1.0 / sz;
            float m2 = m1 + m1;
            float m3 = 1.0 - m1 - m2;
            (*v)->newPoint = new Vertex();
            (*v)->newPoint->pos = ((*v)->pos * m3)
                + (edge_average->pos * m2)
                + (face_average->pos * m1);

        }
        else { // boundary vertex
            (*v)->newPoint = avg_border_edge_midpts(*v);
        }

        mesh->glvertices.push_back((*v)->newPoint);

    }
}

// step (3) generate new vertices
/*
void generate_new_vertices(std::vector<Vertex*> &overtices) { // original vertices in the mesh

    for (std::vector<Vertex*>::iterator i = overtices.begin(); i != overtices.end(); i++) {
        Vertex *v = *i;

        // v' = (Q + 2R + (n-3)*v) / n
        // Q = average of face points, R = new edge points, n = the valence of v
        // boundary v' = (v/2 + m/4) where m is just sum of edge points

        HalfEdge* edge = v->edge;
        Vertex face_average;
        Vertex edge_average;

        // M is the sum of the two midpoints (equivalently the edge points) on the two boundary edges
        if (edge->pair == NULL) {
            printf("boundary edge vertex calculation:\n");
            //edge_average.pos = edge->edgePoint->pos + edge->next->edgePoint->pos;
            //v->pos = (v->pos / 2) + edge_average.pos / 4;
            v = boundary_edgepoint_average(v);
            continue; // for now
        }

        // loop around the vertex!(edge, edge->pair or edge->face) // flipcode
        int valence = 0;
        do {
            valence++;
            face_average.pos = face_average.pos + edge->face->facePoint->pos;
            edge_average.pos = edge_average.pos + edge->edgePoint->pos;
            if (edge->pair == NULL) {break;}
            edge = edge->pair->next;
        } while (edge != v->edge);

        edge_average.pos = edge_average.pos / valence;
        face_average.pos = face_average.pos / valence;
        v->pos = ((v->pos * (valence - 3)) +  (edge_average.pos * 2) + face_average.pos) / valence;
        printf("vertex regular case:\n");
        v->pos.print();
    }
}
 */


void connect_new_mesh(Mesh *mesh, Mesh *previous, std::vector<Vector3f> &draw_vertices) {
    int index = 0;
    for (std::vector<Face*>::iterator i = previous->glfaces.begin(); i != previous->glfaces.end(); i++) {
        HalfEdge *edge = (*i)->edge;
        // loop thru the face's edges
        int num = 0;
        //printf("face %d\n", num++);
        do {
            // the new four vertices for the new face
            std::vector<Vertex*> fvertices;
            fvertices.push_back((*i)->facePoint);
            //(*i)->facePoint->pos.print();

            fvertices.push_back(edge->edgePoint);
            //edge->edgePoint->pos.print();

            fvertices.push_back(edge->next->start->newPoint);
            //edge->next->start->newPoint->pos.print();

            fvertices.push_back(edge->next->edgePoint);
            //edge->next->edgePoint->pos.print();
            //printf("\n");

            std::vector<int> f;
            f.push_back(index++);
            f.push_back(index++);
            f.push_back(index++);
            f.push_back(index++);
            mesh->vertex_indices.push_back(f);

            //draw_vertices.push_back((*i)->facePoint->pos);
            //draw_vertices.push_back(edge->edgePoint->pos);
            //draw_vertices.push_back(edge->next->start->newPoint->pos);
            //draw_vertices.push_back(edge->next->edgePoint->pos);

            make_face(fvertices, mesh);

            edge = edge->next;
            num++;
        } while (edge != (*i)->edge);
    }

/*
    for (std::vector<HalfEdge*>::iterator i = new_halfedges.begin(); i != new_halfedges.end(); i++) {
        for (std::vector<HalfEdge*>::iterator j = i+1; j != new_halfedges.end(); j++) {
            if ((equal((*i)->start->pos, (*j)->end->pos) && equal((*i)->end->pos, (*j)->start->pos))) {
                (*i)->pair = *j;
                (*j)->pair = *i;
            }
        }
    }
 */
}

// step (4) connect
/*
 void connect_new_mesh(std::vector<Face*> &faces,
     std::vector<Face*> &new_faces,
     std::vector<HalfEdge*> &new_halfedges) {

     for (std::vector<Face*>::iterator i = faces.begin(); i != faces.end(); i++) {
         HalfEdge *edge = (*i)->edge;

         // loop thru the face's edges
         int num = 0;
         do {
             // the new four vertices for the new face
             std::vector<Vertex*> fvertices;
             fvertices.push_back(edge->edgePoint);
             fvertices.push_back(edge->start);
             fvertices.push_back(edge->next->edgePoint);
             fvertices.push_back((*i)->facePoint);
             make_face(fvertices, new_faces, new_halfedges);
             edge = edge->next;
             num++;
         } while (edge != (*i)->edge);
     }

     for (std::vector<HalfEdge*>::iterator i = new_halfedges.begin(); i != new_halfedges.end(); i++) {
         for (std::vector<HalfEdge*>::iterator j = i+1; j != new_halfedges.end(); j++) {
             if ((equal((*i)->start->pos, (*j)->end->pos) &&
                  equal((*i)->end->pos, (*j)->start->pos))) {
                 (*i)->pair = *j;
                 (*j)->pair = *i;
             }
         }
     }

     int lonely = 0, partner = 0;
     for (std::vector<HalfEdge*>::iterator i = new_halfedges.begin(); i != new_halfedges.end(); i++) {
         if ((*i)->pair == NULL) {
             lonely++;
        } else {
            partner++;
        }
     }
     printf("lonely = %d partner = %d\n", lonely, partner);
 }
*/
