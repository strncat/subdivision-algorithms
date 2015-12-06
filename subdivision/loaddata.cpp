//
//  loaddata.cpp
//  Project
//
//  Created by Fatima Broom on 8/1/15.
//  Copyright (c) 2015 nemo. All rights reserved.
//

// From Assignment 5

#include "halfedge.h"
#include <fstream>
#include <sstream>

bool Mesh::load_file(const std::string& filename) {
    // peek at the extension
    std::string::size_type idx;
    idx = filename.rfind('.');

    if (idx != std::string::npos) {
        std::string extension = filename.substr(idx + 1);
        if (extension == "obj") {
            return load_object(filename);
        } else {
            std::cerr << "ERROR: unable to load file " << filename
            << "  -- unknown extension." << std::endl;
            std::cerr << "Input only (.obj) files" << std::endl;
        }
    }
    // No filename extension found, or none matching {.obj}
    return false;
}

bool Mesh::check_index(int index, size_t size) {
    if (index < 0 || index >= size) {
        std::cerr << "ERROR: unable to load triangles file; index out of range (" << index << ")" << std::endl;
        return false;
    }
    return true;
}

bool Mesh::load_object(const std::string& filename) {
    std::ifstream ifs(filename.c_str());
    std::vector<Vector3f> temp_vertices;
    std::vector<Vector3f> temp_normals;
    std::vector<Vector3f> temp_textures;

    if (ifs.is_open()) {
        int t0[3], t1[3], t2[3], t3[3];
        float v0, v1, v2;
        std::string line;
        char temp[2];

        while (std::getline(ifs, line)) {
            // (1) load vertices
            if (line[0] == 'v' && line[1] == ' ') { // vertices
                sscanf(line.c_str(), "%s %f %f %f", temp, &v0, &v1, &v2);
                Vector3f vector(v0, v1, v2);
                temp_vertices.push_back(vector);
                Vertex *v = new Vertex(vector);
                glvertices.push_back(v);

                // (2) load normals
            } else if (line[0] == 'v' && line[1] == 'n') { // normals
                sscanf(line.c_str(), "%s %f %f %f", temp, &v0, &v1, &v2);
                Vector3f vector(v0, v1, v2);
                temp_normals.push_back(vector);

                // (3) load textures
            } else if (line[0] == 'v' && line[1] == 't') { // textures
                sscanf(line.c_str(), "%s %f %f %f", temp, &v0, &v1, &v2);
                Vector3f vector(v0, v1, v2);
                temp_textures.push_back(vector);

                // (4) load faces - usually last part of the file
            } else if (line[0] == 'f') {
                t0[0] = t0[1] = t0[2] = t1[0] = t1[1] = t1[2] = t2[0] = t2[1] = t2[2] = t3[0] = t3[1] = t3[2] = -1;

                if (line.find("//") != std::string::npos) { // vertices and normals
                    sscanf(line.c_str(), "%s %d//%d %d//%d %d//%d %d//%d",
                           temp, &t0[0], &t0[2],
                           &t1[0], &t1[2],
                           &t2[0], &t2[2],
                           &t3[0], &t3[2]);

                } else if (line.find("/") != std::string::npos) { // everything or textures
                    int total = sscanf(line.c_str(), "%s %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                                       temp, &t0[0], &t0[1], &t0[2],
                                       &t1[0], &t1[1], &t1[2],
                                       &t2[0], &t2[1], &t2[2],
                                       &t3[0], &t3[1], &t3[2]);

                    // sscanf returns the number of values it was able to parse
                    if (total < 10) { // only vertices and textures
                        sscanf(line.c_str(), "%s %d/%d %d/%d %d/%d %d/%d", temp, &t0[0],
                               &t0[1], &t1[0], &t1[1], &t2[0], &t2[1], &t3[0], &t3[1]);
                    }

                } else { // no '/' so only vertices
                    sscanf(line.c_str(), "%s %d %d %d %d", temp, &t0[0], &t1[0], &t2[0], &t3[0]);
                }

                std::vector<int> face;
                face.push_back(t0[0]-1);
                face.push_back(t1[0]-1);
                face.push_back(t2[0]-1);


                if (t3[0] != -1) {
                    quad = true;
                    face.push_back(t3[0]-1);
                }
                vertex_indices.push_back(face);

                if (t0[1] != -1 && t1[1] != -1) {
                    hasTextures = true;
                    //v1->texture = temp_textures[t0[1]-1];
                    //v2->texture = temp_textures[t2[1]-1];
                }

                if (t0[2] != -1 && t1[2] != -1) {
                    //v1->normal = temp_normals[t0[2]-1];
                    //v2->normal = temp_normals[t1[2]-1];
                    if (t2[2] != -1) {
                        //v3->normal = temp_normals[t2[2]-1];
                    }
                }
            }
        }
    }
    return true;
}
