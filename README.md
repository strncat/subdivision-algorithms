Implementation of Subdivison Algorithms
==============

#### Mesh Representation : Half Edge Data Structure ####

<p align="center"><img src="https://github.com/fbroom/subdivision-algorithms/blob/master/images/ds.png" width="300"></p>

A flexible and fast data structure is needed to implement any subdivision algorithm since we need the ability to make queries like finding all the adjacent faces to a vertex or all the vertices in a face. The half edge data structure was used for this project. It is combact with no redundancy. It also provides linear time access to all the adjaceny information. It is an adge based data structure where all the connectivity information is stored in the half edge. Both Pixar’s OpenSubDiv and OpenMesh (http://www.openmesh.org/Daily-Builds/Doc/a00016.html) use the half edge data structure.

#### Outline ####

A mesh contains the following:

```
class Mesh {
    std::vector<HalfEdge*> glhalfedges;
    std::vector<Vertex*> glvertices;
    std::vector<Face*> glfaces;
}
```

* Vector of vertices where each vertex references the the outgoing half edge that starts at this vertex

```
struct Vertex {
    Vector3f pos;
    Vector3f normal;
    Vector3f texture;
    HalfEdge *edge;
    // more properties
};
```

* Vector of faces where each face references one of the half edges belonging to that faces

```
struct Face {
    HalfEdge *edge;
    Vertex *facePoint;
};
```

* a vector of half edges where each half edge references:
 * The vertex it is pointing at
 * The face it belongs to
 * The next half edge (counter clockwise)
 * The opposite or twin/pair half edge
 * For my implementation I also added a pointer to the previous edge which will make traversal easier for finding the boundary edges of a vertex

```
struct HalfEdge {
    Vertex *start; // the Vertex it points to
    HalfEdge *next; // the next HalfEdge inside the face
    HalfEdge *prev; // the prev HalfEdge
    HalfEdge *pair; // the pair HalfEdge
    Face *face; // the face it belongs to
};
```

#### Traversal ####

* Traversing vertices in a face
<p align="center"><img src="https://github.com/fbroom/subdivision-algorithms/blob/master/images/ds-faces.png" width="300"></p>
```
HalfEdge *edge = face->edge; // each faces points to an edge 
do {
    edge->start = ... // perform an operation on the vertex
    edge = edge->next;
} while (edge != face->edge);
```

* Traversing adjacent faces
<p align="center"><img src="https://github.com/fbroom/subdivision-algorithms/blob/master/images/ds-vertex.png" width="300"></p>
```
HalfEdge *edge = vertex ->edge; 
do {
    // edge->face // perform an operation on the face
    edge = edge->pair->next;
} while (edge != vertex->edge);
```


* Traversing a boundary vertex



## Catmull-Clark Subdivision ##

#### Algorithm ####

![alt tag](http://url/to/img.png)

1. For each face in the mesh, we generate a face point F, which is just the average of the points in that face. See figure above

2. For each edge in the mesh, we generate a new edge point E, which is the average of the two vertices connected to the edge plus the two new face points adjacent to the edge. If the edge is a boundary edge then the edge point is just the average of the two vertices connected to the edge

3. For each vertex v in the mesh, we calculate a new vertex
    v': v' =(−Q + 4*R + S * (n−3))/n

where Q, R and S are:
 a. Q is the average of the new face points of all the faces connected to the vertex 
 b. R is the average of the new edge points of all the edges connected o the vertex
 c. S is the given vertex and n is its valence

If the vertex was a boundary vertex then the rule is simply:
    v' = S/2 + M/4 

where S and M are:
 a. M is the average of the new edge points of all the edges connected o the vertex
 b. S is the given vertex

4. For each face in the mesh we connect the face points calculated in step 1 to the new edge points calculated in step 2

5. We connect each new vertex v' created in step 3 to the new edge points created in step 2

#### Results ####

<p align="center">
<img src="https://github.com/fbroom/subdivision-algorithms/blob/master/results/catmull/donut/0.png" width="300">
<img src="https://github.com/fbroom/subdivision-algorithms/blob/master/results/catmull/donut/1.png" width="300">
<img src="https://github.com/fbroom/subdivision-algorithms/blob/master/results/catmull/donut/2.png" width="300">
<img src="https://github.com/fbroom/subdivision-algorithms/blob/master/results/catmull/donut/3.png" width="300">
<img src="https://github.com/fbroom/subdivision-algorithms/blob/master/results/catmull/donut/4.png" width="300">
<img src="https://github.com/fbroom/subdivision-algorithms/blob/master/results/catmull/donut/5.png" width="300">
<img src="https://github.com/fbroom/subdivision-algorithms/blob/master/results/catmull/donut/6.png" width="300">
<img src="https://github.com/fbroom/subdivision-algorithms/blob/master/results/catmull/donut/7.png" width="300">
</p>

## Loop Subdivision ##

1. For each edge in the mesh, we generate an edge point E, which is a weighted average of the points in in the figure above. If the edge is a boundary edge then edge point is just the average of the two vertices connected to the edge

2. For every vertex in the mesh, we generate a new vertex location calculated as a weighted average of the vertices connected to the vertex by an edge and the vertex itself:

v' = n * b * (sum of connectedV) +(1 − n*b)*v 

where b = 3 if n > 3 and b = 3/16 if n = 3

if n = 2 which simply means that the vertex is a boundary vertex, the new vertex will have weighted average of the connected two connected vertices:

v' = 1/4 * (sum of connectedV) + 3/4 * v




## Butterfly Subdivision ##
to be added

