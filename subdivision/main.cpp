//
//  main1.cpp
//  Project
//
//  Created by Fatima B on 7/24/15.
//  Copyright (c) 2015 nemo. All rights reserved.
//


#include "main.h"
#ifdef WIN32
#define ssize_t SSIZE_T
#endif

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cmath>
#include "halfedge.h"
#include "catmullclark.h"
#include "loop.h"
#include "butterfly.h"
#include <time.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define NUM_LIGHTS 2
glm::vec3 lightPosition[NUM_LIGHTS] = {glm::vec3(10,10,10), glm::vec3(50,50,50)};
std::string g_filename;
glm::vec3 color;
glm::vec3 Ka;
glm::vec3 Kd;
glm::vec3 Ks;

float g_translateX = 0.0;
float g_translateY = 0.0;
float g_translateZ = 0.0;

float g_x_degrees = 0.0;
float g_y_degrees = 0.0;
float g_z_degrees = 0.0;

int option;

std::vector<Mesh*> meshes;
int g_iteration = 0;

std::vector<Vector3f> draw_vertices;
std::vector<Vector3f> draw_normals;

GLuint vbo_buffer, vbo_indices, v_position, v_normal;
int g_vsize, g_nsize, g_tsize, g_isize;

// file locations
std::string vertexShader;
std::string fragmentShader;
std::string geometryShader;
bool geometryShaderEnable = false;
int geometryShaderOutputType = 0; // triangles

SimpleShaderProgram *shader;

void catmull_clark();
void catmull_clark_subdivision();
void loop();
void loop_subdivision();
void buttefly();
void buttefly_subdivision();

void screenshot() {
    int w = glutGet(GLUT_WINDOW_WIDTH); int h = glutGet(GLUT_WINDOW_HEIGHT);
    float out[3 * w * h];
    RGBColor BG(0,0,0);
    SimpleImage shot(w, h, BG);
    glReadPixels(0, 0, w, h, GL_RGB, GL_FLOAT, &out[0]); for (int y = 0; y < h; ++y){
        for(int x = 0; x < w; ++x){
            int index = (3 * w * y) + 3*x;
            float red = out[index];
            float green = out[index + 1];
            float blue = out[index + 2]; shot.set(x, h - y,RGBColor(red, green, blue));
        }
    }
    shot.save("screenshot.png");
}

void DrawWithShader() {
    shader->Bind();

    // (1) projection is perspective
    int w = glutGet(GLUT_WINDOW_WIDTH); int h = glutGet(GLUT_WINDOW_HEIGHT);
    glm::mat4 projectionMatrix = glm::perspective(30.0f, (float)w/(float)h, 0.1f, 100000.f);

    // (2) view matrix
    glm::vec3 eyePosition = glm::vec3(0,0,4);
    glm::vec3 center = glm::vec3(0,0,0);
    glm::vec3 up = glm::vec3(0,1,0);
    glm::mat4 viewMatrix = glm::lookAt(eyePosition, // eye position
                                       center, // center looking at
                                       up); // up

    // (3) model matrix
    glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(g_translateX, g_translateY, g_translateZ));
    glm::mat4 rotateX = glm::rotate(glm::mat4(1), glm::radians(g_x_degrees), glm::vec3(1,0,0));
    glm::mat4 rotateY = glm::rotate(glm::mat4(1), glm::radians(g_y_degrees), glm::vec3(0,1,0));
    glm::mat4 rotateZ = glm::rotate(glm::mat4(1), glm::radians(g_z_degrees), glm::vec3(0,0,1));

    // model, modelView, modelViewProjection & normal matrices
    glm::mat4 modelMatrix = translate * rotateZ * rotateY * rotateX;
    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    glm::mat4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelViewMatrix)));

    // transfer to the GPU
    GLuint eyeL = glGetUniformLocation(shader->programid, "eyePosition");
    glUniform3fv(eyeL, 1, &eyePosition[0]);
    inline_gl_error("eyePosition");

    GLuint lightL = glGetUniformLocation(shader->programid, "lightPosition");
    glUniform3fv(lightL, NUM_LIGHTS, &lightPosition[0][0]);
    inline_gl_error("lightPosition");

    GLuint colorL = glGetUniformLocation(shader->programid, "color");
    GLuint KaL = glGetUniformLocation(shader->programid, "Ka");
    GLuint KdL = glGetUniformLocation(shader->programid, "Kd");
    GLuint KsL = glGetUniformLocation(shader->programid, "Ks");
    glUniform3fv(colorL, 1, &color[0]);
    glUniform3fv(KaL, 1, &Ka[0]);
    glUniform3fv(KdL, 1, &Kd[0]);
    glUniform3fv(KsL, 1, &Ks[0]);
    inline_gl_error("color");

    GLuint normL = glGetUniformLocation(shader->programid, "normalMatrix");
    glUniformMatrix3fv(normL, 1, GL_FALSE, &normalMatrix[0][0]);
    inline_gl_error("normalMatrix");

    GLuint mvpL = glGetUniformLocation(shader->programid, "modelViewProjectionMatrix");
    glUniformMatrix4fv(mvpL, 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);
    inline_gl_error("modelViewProjectionMatrix");

    if (geometryShaderOutputType == 0) {
        if (meshes[g_iteration]->quad) {
            glDrawArrays(GL_QUADS, 0, (int)draw_vertices.size());
        } else {
            glDrawArrays(GL_TRIANGLES, 0, (int)draw_vertices.size());
        }
    } else if (geometryShaderOutputType == 1) {
        glDrawArrays(GL_TRIANGLES_ADJACENCY_EXT, 0, (int)draw_vertices.size());
    }
    inline_gl_error("drawArrays");

    shader->UnBind();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawWithShader();
    glutSwapBuffers();
    screenshot();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

// only used for the initial mesh, normals for subdivision are calculated there
void calculate_normals() {
    draw_normals.clear();
    int inc = meshes[g_iteration]->quad ? 4 : 3;
    for (int i = 0; i < draw_vertices.size(); i+=inc) {
        // each three will make a face
        Vector3f a = draw_vertices[i];
        Vector3f b = draw_vertices[i+1];
        Vector3f c = draw_vertices[i+2];
        Vector3f d;

        if (meshes[g_iteration]->quad) {
            d = draw_vertices[i+3];

        }
            // find for each vertex
        Vector3f ab = a - b;
        Vector3f bc = b - c;
        Vector3f normal;
        cross(&normal, &ab, &bc);
        normal.normalize();
        draw_normals.push_back(normal);
        draw_normals.push_back(normal);
        draw_normals.push_back(normal);

        if (meshes[g_iteration]->quad) {
            draw_normals.push_back(normal);
        }
    }
}

void setup_vbos() {
    g_vsize = (int)draw_vertices.size() * sizeof(Vector3f);
    g_nsize = (int)draw_normals.size() * sizeof(Vector3f);

    // create the vbo buffer
    glGenBuffers(1, &vbo_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_buffer);
    // just allocate the memory for now
    glBufferData(GL_ARRAY_BUFFER, g_vsize + g_nsize + g_tsize, NULL, GL_DYNAMIC_DRAW);

    // add the data in the buffers
    glBufferSubData(GL_ARRAY_BUFFER, 0, g_vsize, &draw_vertices[0]); // vertices
    glBufferSubData(GL_ARRAY_BUFFER, g_vsize, g_nsize, &draw_normals[0]); // normals

    // get the location of the attributes
    v_position = glGetAttribLocation(shader->programid, "in_position");
    inline_gl_error("get attribute for in_position");

    v_normal = glGetAttribLocation(shader->programid, "in_normal");
    inline_gl_error("get attribute for in_normal");

    glEnableVertexAttribArray(v_position);
    inline_gl_error("enable attribute for v_position");

    glVertexAttribPointer(v_position,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vector3f),
                          0);
    inline_gl_error("vertex pointer for in_position");


    glEnableVertexAttribArray(v_normal);
    inline_gl_error("enable attribute for v_normal");

    glVertexAttribPointer(v_normal,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vector3f),
                          BUFFER_OFFSET(g_vsize));
    inline_gl_error("vertex pointer for v_normal");
}

void setup_gl() {
    shader = new SimpleShaderProgram();
    shader->LoadVertexShader(vertexShader);
    shader->LoadFragmentShader(fragmentShader);

    if (geometryShaderEnable) {
        shader->geometry_shader_enabled = true;
        shader->geometry_out_type = geometryShaderOutputType;
        shader->LoadGeometryShader(geometryShader);
    }
    shader->attached_and_link_shaders();

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    // culling
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void print_draw() {
    int inc = meshes[g_iteration]->quad ? 4 : 3;
    for (int i = 0; i < draw_vertices.size(); i+=inc) {
        printf("v ");
        draw_vertices[i].print();
        printf("v ");
        draw_vertices[i+1].print();
        printf("v ");
        draw_vertices[i+2].print();

        //draw_normals[i].print();
        //draw_normals[i+1].print();
        //draw_normals[i+2].print();

        if (inc == 4) {
            draw_vertices[i+3].print();
            //draw_normals[i+3].print();
        }
        printf("\n");
    }
}

void print_indices() {
    int inc = meshes[g_iteration]->quad ? 4 : 3;
    for (int i = 0; i < draw_vertices.size(); i+=inc) {
        printf("f %d %d %d\n", i+1, i+2, i+3);
    }
}

void prepare_to_draw(int it) {
    draw_vertices.clear();
    draw_normals.clear();
    if (it < 0 || it > g_iteration) {return;}
    for (std::vector<Face*>::iterator f = meshes[it]->glfaces.begin();
         f != meshes[it]->glfaces.end(); f++) {
        HalfEdge *e = (*f)->edge;
        do {
            draw_vertices.push_back(e->start->pos);
            draw_normals.push_back(e->start->normal);
            e = e->next;
        } while (e != (*f)->edge);
    }
}

// Catmull-Clark!!!!!!!!
void catmull_clark_subdivision() {
    generate_face_points(meshes[g_iteration], meshes[g_iteration-1]); // 1
    generate_edge_points(meshes[g_iteration], meshes[g_iteration-1]); // 2
    generate_new_vertices(meshes[g_iteration], meshes[g_iteration-1]); // 3
    connect_new_mesh(meshes[g_iteration], meshes[g_iteration-1]); // 4
}

void catmull_clark() {
    Mesh *mesh = new Mesh(); // create a new mesh
    mesh->quad = true; // catmull generates quads
    meshes.push_back(mesh); // add the new mesh
    g_iteration++;

    printf("-------------------start %d--------------------\n", g_iteration);
    clock_t begin, end; double time_spent; begin = clock();
    catmull_clark_subdivision(); // perform the subdivision
    end = clock(); time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("-------------------done %.3f-------------------\n", time_spent);


    prepare_to_draw(g_iteration); // copy mesh vertices into draw_arrays
    setup_vbos(); // copy draw_arrays into vbos
}

void loop_subdivision() {
    loop_generate_edge_points(meshes[g_iteration], meshes[g_iteration-1]); // 1
    loop_generate_new_vertices(meshes[g_iteration], meshes[g_iteration-1]); // 2
    loop_connect_new_mesh(meshes[g_iteration], meshes[g_iteration-1]); // 3
}

void loop() {
    Mesh *mesh = new Mesh();
    mesh->quad = false;
    meshes.push_back(mesh);
    g_iteration++;

    printf("-------------------start %d--------------------\n", g_iteration);
    clock_t begin, end; double time_spent; begin = clock();
    loop_subdivision();
    end = clock(); time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("-------------------done %.3f-------------------\n", time_spent);


    prepare_to_draw(g_iteration);
    //print_draw();
    //print_indices();
    setup_vbos();
}

void buttefly_subdivision() {
    butterfly_generate_edge_points(meshes[g_iteration], meshes[g_iteration-1]); // 1
    butterfly_connect_new_mesh(meshes[g_iteration], meshes[g_iteration-1]); // 3
}

void buttefly() {
    Mesh *mesh = new Mesh();
    mesh->quad = false;
    meshes.push_back(mesh);
    g_iteration++;

    printf("-------------------start %d--------------------\n", g_iteration);
    clock_t begin, end; double time_spent; begin = clock();
    buttefly_subdivision();
    end = clock(); time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("-------------------done %.3f-------------------\n", time_spent);


    prepare_to_draw(g_iteration);
    print_draw();
    //print_indices();
    setup_vbos();
}

void init_data() {
    g_iteration = 0;
    Mesh *mesh = new Mesh();
    meshes.push_back(mesh);

    if (!meshes[g_iteration]->load_file(g_filename)) { // load data
        printf("Mesh file couldn't be laoded\n");
        exit(0);
    }
    // create the half edge data structure and load the draw array
    init_mesh(meshes[g_iteration], draw_vertices);

    calculate_normals();
    //print_draw();
}


void keyboard(unsigned char key, int x, int y) {
    switch (key) {
            // translate in the z direction
        case 'l':
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case 'f':
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // temp
            break;
        case 'k':
            catmull_clark();
            break;
        case 'p':
            loop();
            break;
        case 'b':
            buttefly();
            break;
        case '[':
            prepare_to_draw(g_iteration-1);
            setup_vbos();
            break;
        case ']':
            prepare_to_draw(g_iteration+1);
            setup_vbos();
            break;
        case '=':
        case '+':
            g_translateZ+=.1;
            break;
        case '-':
            g_translateZ-=.1;
            break;
            // rotate around the z axis
        case 'x':
            g_x_degrees+=10.0;
            break;
        case 'c':
            g_x_degrees-=10.0;
            break;
            // rotate around the y axis
        case 'y':
            g_y_degrees+=10.0;
            break;
        case 'u':
            g_y_degrees-=10.0;
            break;
            // rotate around the z axis
        case 'z':
            g_z_degrees+=10.0;
            break;
        case 'a':
            g_z_degrees-=10.0;
            break;
        case '1':
            color = glm::vec3(1.0, 0.0, 0.0);
            break;
        case '2':
            color = glm::vec3(0.0, 1.0, 0.0);
            break;
        case '3':
            color = glm::vec3(0.0, 0.0, 1.0);
            break;
        case '4':
            color = glm::vec3(1.0, 1.0, 0.0);
            break;
        case '5':
            color = glm::vec3(0.0, 1.0, 1.0);
            break;
        case '6':
            color = glm::vec3(1.0, 0.0, 1.0);
            break;
        case '7':
            color = glm::vec3(0.682, 0.592, 0.99);
            break;
        case '8':
            color = glm::vec3(0.7, 0.7, 0.7);
            break;
    }
    glutPostRedisplay();
}

void keyboard_special(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    // key up
            g_translateY-=.3;
            break;
        case GLUT_KEY_DOWN:    // key down
            g_translateY+=.3;
            break;
        case GLUT_KEY_RIGHT:    // key right
            g_translateX-=.3;
            break;
        case GLUT_KEY_LEFT:    // key left
            g_translateX+=.3;
            break;
        case 27:
            exit (0);
            break;
    }
    glutPostRedisplay();
}

void mouse_click(int button, int state, int x, int y) {
    switch (button) {
        case GLUT_LEFT_BUTTON:
            g_translateZ+=.1;
            break;
        case GLUT_RIGHT_BUTTON:
            g_translateZ-=.1;
            break;
    }
    glutPostRedisplay();
}

void menu() {
    printf("0 to generate a quad\n");
    printf("1 to generate a triangle\n");
    printf("2 to generate venus\n");
    printf("3 to generate cow\n");
    printf("4 to generate bunny\n");
    printf("5 to generate teddy bear\n");
    printf("6 to generate stell.obj\n");
    printf("7 to generate ico.obj\n");
    printf("8 to generate stell.obj\n");


}

void user_input() {
    printf("************************************************\n");
    printf("l to render with wireframe and f to switch back\n");
    printf("k to subdivide using catmull-clark\n");
    printf("p to subdivide using loop\n");
    printf("b to subdivide using the butterfly scheme\n");
    printf("+ - to zoom in and out (translate in the z direction)\n");
    printf("(keypad arrows) to translate in the x and y directions\n");
    printf("x and c to rotate around the x axis\n");
    printf("y and u to rotate around the y axis\n");
    printf("z and a to rotate around the z axis\n");
    printf("1 to 8 change the color of the mesh\n");
    printf("************************************************\n");
}

int main(int argc, char** argv) {
    /*
     if (!(argc == 2)) {
     printf("usage: ./hw5 option \n");
     menu();
     return 0;
     } */
    int option = 2;
    menu();
    user_input();

    vertexShader = "phong.vert";
    fragmentShader = "phong.frag";
    geometryShaderEnable = false;
    geometryShaderOutputType = 0;
    Ka = glm::vec3(0.2f, 0.2f, 0.2f); // ambient constant
    Kd = glm::vec3(0.5f, 0.5f, 0.5f); // diffuse
    Ks = glm::vec3(1.0f, 1.0f, 1.0f); // specular

    if (option == 0) {
        g_filename = "quad.obj";
        color = glm::vec3(0.682, 0.592, 0.99);
        g_translateX = -1.5; g_translateY = -1.5; g_translateZ = 0.0;
        g_x_degrees = 0.0; g_y_degrees = 0.0; g_z_degrees =  0.0;
    }
    else if (option == 1) {
        g_filename = "triangle.obj";
        color = glm::vec3(0.682, 0.592, 0.99);
        g_translateX = 0.0; g_translateY = -1.5; g_translateZ = 1.0;
        g_x_degrees = 0.0; g_y_degrees = 0.0; g_z_degrees =  0.0;
    }
    else if (option == 2) {
        g_filename = "venus.obj";
        Ks = glm::vec3(0.0f, 0.0f, 0.0f); // specular
        color = glm::vec3(0.682, 0.592, 0.99);
        g_translateX = 0.0; g_translateY = 1.0; g_translateZ = -3.0;
        g_x_degrees = -90.0; g_y_degrees = 0.0; g_z_degrees = 170.0;
    }
    else if (option == 3) { // cow
        g_filename = "cow.obj";
        color = glm::vec3(.212, .714, .659);
        g_translateX = 0.0; g_translateY = 0.0; g_translateZ = 3.0;
        g_x_degrees = 180.0; g_y_degrees = 0.0; g_z_degrees = 0.0;
    }
    else if (option == 4) { // bunny
        g_filename = "bunny.obj";
        color = glm::vec3(1.0, 0.2, 0.3);
        g_translateX = 0.0; g_translateY = 0.15; g_translateZ = 3.7;
        g_x_degrees = 10.0; g_y_degrees = 190.0; g_z_degrees = 180.0;
    }
    else if (option == 5) { // teddy
        g_filename = "teddy.obj";
        color = glm::vec3(1.0, 0.2, 0.3);
        g_translateX = 0.0; g_translateY = 0.15; g_translateZ = -40.0;
        g_x_degrees = 0.0; g_y_degrees = 0.0; g_z_degrees = 180.0;
    }
    else if (option == 6) {
        g_filename = "stell.obj";
        color = glm::vec3(0.682, 0.592, 0.99);
        g_translateX = 0.0; g_translateY = -1.5; g_translateZ = 1.0;
        g_x_degrees = 0.0; g_y_degrees = 0.0; g_z_degrees =  0.0;
    }
    else if (option == 7) {
        g_filename = "ico.obj";
        color = glm::vec3(0.682, 0.592, 0.99);
        g_translateX = 0.0; g_translateY = -1.5; g_translateZ = 1.0;
        g_x_degrees = 0.0; g_y_degrees = 0.0; g_z_degrees =  0.0;
    }
    else if (option == 8) {
        g_filename = "torus.obj";
        color = glm::vec3(0.682, 0.592, 0.99);
        g_translateX = 0.0; g_translateY = -1.5; g_translateZ = -2.0;
        g_x_degrees = 0.0; g_y_degrees = 0.0; g_z_degrees =  0.0;
    }

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(1800, 20);
    glutInitWindowSize(640, 480);
    glutCreateWindow("CS148 Project");

    
    // Initialize GLEW
#if !defined(__APPLE__) && !defined(__linux__)
    glewInit();
    if(!GLEW_VERSION_2_0) {
        printf("Your graphics card or graphics driver does\n"
               "\tnot support OpenGL 2.0, trying ARB extensions\n");
        
        if(!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader) {
            printf("ARB extensions don't work either.\n");
            printf("\tYou can try updating your graphics drivers.\n"
                   "\tIf that does not work, you will have to find\n");
            printf("\ta machine with a newer graphics card.\n");
            exit(1);
        }
    }
#endif

    // initial setup
    setup_gl();
    init_data();
    setup_vbos();

    // display & reshape
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(display);
    
    // user input
    glutSpecialFunc(keyboard_special);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse_click);
    
    glutMainLoop();
    return 0;
}
