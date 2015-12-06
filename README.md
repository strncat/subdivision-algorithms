Implementation of Subdivison Algorithms
==============

#### Mesh Representation : Half Edge Data Structure ####

In order to be able to implement the subdivision algorithms, a flexible and fast data structure was needed. We need to be 
able to query all the adjacent faces to a vertex or all the vertices in face for example. The half edge data structure was used for this project.
It is an adge based data structure where all the connectivity information is stored in the half edge. 
Both Pixar’s OpenSubDiv and OpenMesh use the half edge data structure 

# Outline

```
class Mesh {
    std::vector<HalfEdge*> glhalfedges;
    std::vector<Vertex*> glvertices;
    std::vector<Face*> glfaces;
}
```

1. Vertex: references the the outgoing half edge that starts at this vertex

```
struct Vertex {
    Vector3f pos;
    Vector3f normal;
    Vector3f texture;
    HalfEdge *edge;
    // more properties
};
```

2. Face: references one of the half edges belonging to that faces

```
struct Face {
    HalfEdge *edge;
    Vertex *facePoint;
};
```

3. Half Edge: references:
 a. The vertex it is pointing at
 b. The face it belongs to
 c. The next half edge (counter clockwise)
 d. The opposite or twin/pair half edge
 e. For my implementation I also added a pointer to the previous edge which will make 
traversal easier for finding the boundary edges of a vertex

```
struct HalfEdge {
    Vertex *start; // the Vertex it points to
    HalfEdge *next; // the next HalfEdge inside the face
    HalfEdge *prev; // the prev HalfEdge
    HalfEdge *pair; // the pair HalfEdge
    Face *face; // the face it belongs to
};
```

# Traversal

1. Traversing vertices in a face

2. Traversing adjacent faces

3. Traversing a boundary vertex



#### Catmull-Clark Subdivision ####

# Algorithm

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

# Results



#### Loop Subdivision ####

1. For each edge in the mesh, we generate an edge point E, which is a weighted average of the points in in the figure above. If the edge is a boundary edge then edge point is just the average of the two vertices connected to the edge

2. For every vertex in the mesh, we generate a new vertex location calculated as a weighted average of the vertices connected to the vertex by an edge and the vertex itself:

v' = n * b * (sum of connectedV) +(1 − n*b)*v 

where b = 3 if n > 3 and b = 3/16 if n = 3

if n = 2 which simply means that the vertex is a boundary vertex, the new vertex will have weighted average of the connected two connected vertices:

v' = 1/4 * (sum of connectedV) + 3/4 * v




#### Butterfly Subdivision ####
to be added

