//
//  catmullclark.cpp
//  Project
//
//  Created by Fatima Broom on 8/3/15.
//  Copyright (c) 2015 nemo. All rights reserved.
//

#include "catmullclark.h"

// (1)
void generate_face_points(Mesh *mesh, Mesh *previous) {
    for (std::vector<Face*>::iterator f = previous->glfaces.begin();
         f != previous->glfaces.end(); f++) {

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
    for (std::vector<HalfEdge*>::iterator i = previous->glhalfedges.begin();
         i < previous->glhalfedges.end(); i++) {
        HalfEdge **edge = &(*i);
        if ((*edge)->edgePoint) { continue; }

        Vertex *edgePoint = new Vertex(); // make a new edge point

        if ((*edge)->pair == NULL) { // BOUNDARY : Just the average of that edge
            (*edgePoint).pos = ((*edge)->start->pos + (*edge)->next->start->pos) / 2.0;
            (*edge)->edgePoint = edgePoint;

        } else if ((*edge)->pair) { // not boundary edge
            Vector3f edge1 = (*edge)->start->pos;
            Vector3f edge2 = (*edge)->next->start->pos;
            Vector3f face1 = (*edge)->face->facePoint->pos;
            Vector3f face2 = (*edge)->pair->face->facePoint->pos;

            edgePoint->pos = ((face1 + face2)*0.5 + (edge1 + edge2)*0.5) * 0.5;

            // assign to both
            (*edge)->edgePoint = edgePoint;
            (*edge)->pair->edgePoint = (*edge)->edgePoint;
        }

        // IMPORTANT adding (*edge)->edgePoint and NOT edgePoint :(
        mesh->glvertices.push_back((*edge)->edgePoint); // just another vertex
    }
}

Vector3f find_q(Vertex *v) {
    // (1) FIND Q (face average) ----------------
    Vector3f face_average;
    int num_faces = 0;
    HalfEdge* e = v->edge;

    // loop around the vertex // flip code
    do {
        num_faces++;
        face_average = face_average + e->face->facePoint->pos;
        // AGH WE NEED TO GO BACKWARD/CLOCKWISE TO FIND THE REST OF THE FACES
        if (e->pair == NULL) {
            HalfEdge *p = v->edge->prev;
            while (p) {
                num_faces++;
                face_average = face_average + p->face->facePoint->pos;
                p = p->pair ? p->pair->prev : NULL;

            }
            break;
        }
        e = e->pair->next;
    } while (e != v->edge);
    face_average = face_average / num_faces;
    return face_average; // copy
}


// (3) most complicated step :(
void generate_new_vertices(Mesh *mesh, Mesh *previous) { // original vertices in the mesh
    for (std::vector<Vertex*>::iterator i = previous->glvertices.begin();
         i < previous->glvertices.end(); i++) {

        Vertex **v = &(*i);
        (*v)->newPoint = new Vertex();

        // (1) FIND Q (face average) ----------------
        Vector3f face_average = find_q(*v);


        // (2) FIND R (edgepoints average) ----------------
        std::vector<Vertex*> neighbors;
        Vector3f edge_average;
        int valence = 0;

        HalfEdge *e = (*v)->edge;
        do {
            neighbors.push_back(e->edgePoint);
            edge_average = edge_average + e->edgePoint->pos;
            valence++;
            if (e->pair == NULL) {
                HalfEdge *p = (*v)->edge->prev;
                while (p) {
                    neighbors.push_back(p->edgePoint);
                    edge_average = edge_average + p->edgePoint->pos;
                    valence++;
                    p = p->pair ? p->pair->prev : NULL;
                }
                break;
            }
            e = e->pair->next; // next edge
        } while (e != (*v)->edge);
        edge_average = edge_average / valence;


        // v' = (-Q + 4R + (valence-3)*v) / valence
        if (valence > 3) { // REGULAR VERTEX
            (*v)->newPoint->pos = (*v)->pos * (valence - 3.0) + edge_average * 2.0 + face_average;
            (*v)->newPoint->pos = (*v)->newPoint->pos * (1.0/valence);
        }

        else if (valence == 3) { // weird case?????
            Vector3f sum;
            for (std::vector<Vertex*>::iterator t = neighbors.begin(); t != neighbors.end(); t++) {
                sum = sum + (*t)->pos;
            }
            //(*v)->newPoint->pos = sum * 0.25 + (*v)->pos*0.25; // WHY NOT???????????
            HalfEdge *ccw_edge = most_counter_clockwise((*v)->edge);
            HalfEdge *cw_edge = most_clockwise((*v)->edge);
            (*v)->newPoint->pos = (ccw_edge->edgePoint->pos + cw_edge->edgePoint->pos)*.25 + (*v)->pos*0.5;
        }

        // rule for a vertex = S/2 + M/4
        else { // BOUNDARY VERTEX
            for (std::vector<Vertex*>::iterator t = neighbors.begin(); t != neighbors.end(); t++) {
                (*v)->newPoint->pos = (*v)->newPoint->pos + (*t)->pos;
            }
            (*v)->newPoint->pos = (*v)->newPoint->pos * 0.25; // M
            (*v)->newPoint->pos = (*v)->newPoint->pos + ((*v)->pos*0.5);
        }
        // IMPORTANT don't add the newPoint! add (*v)->newPoint :(
        mesh->glvertices.push_back((*v)->newPoint);
    }
}

void connect_new_mesh(Mesh *mesh, Mesh *previous) {
    //int index = 0;
    for (std::vector<Face*>::iterator i = previous->glfaces.begin();
         i != previous->glfaces.end(); i++) {
        HalfEdge *edge = (*i)->edge;

        // loop thru the face's edges // flipcode
        int num = 0;
        do {
            // the new four vertices for the new face
            std::vector<Vertex*> fvertices;

            Vertex **v1 = &((*i)->facePoint);
            Vertex **v2 = &(edge->edgePoint);
            Vertex **v3 = &(edge->next->start->newPoint);
            Vertex **v4 = &(edge->next->edgePoint);

            fvertices.push_back(*v1);
            fvertices.push_back(*v2);
            fvertices.push_back(*v3);
            fvertices.push_back(*v4);

            // update indices & calculate normals and prepare draw vertices all at once
            //std::vector<int> f;
            //f.push_back(index++);
            //f.push_back(index++);
            //f.push_back(index++);
            //f.push_back(index++);
            //mesh->vertex_indices.push_back(f);

            // calculate per face normal
            // begin
            Vector3f ab = (*v1)->pos - (*v2)->pos;
            Vector3f bc = (*v2)->pos - (*v3)->pos;
            Vector3f normal;
            cross(&normal, &ab, &bc);
            normal.normalize();
            (*v1)->normal = normal;
            (*v2)->normal = normal;
            (*v3)->normal = normal;
            (*v4)->normal = normal;
            // end

            make_face(fvertices, mesh);

            edge = edge->next;
            num++;
        } while (edge != (*i)->edge);
    }
}

