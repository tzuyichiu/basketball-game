
#define _USE_MATH_DEFINES

#include <math.h>
#include <random>
#include <iostream>

#include "Projet.hpp"

#ifdef PROJET

using namespace vcl;

// Uniform distribution in [0,1]
std::uniform_real_distribution<float> distrib(0.0, 1.0);
std::default_random_engine generator;

vec3 spring_force(const vec3& pi, const vec3& pj, float L0, float K);
vec3 evaluate_terrain(float u, float v);
mesh create_terrain();
mesh create_cone(float radius, float height, float z_offset);
mesh mesh_primitive_ring(float big_radius, float small_radius, const vec3& pos, const vec3& dir, size_t Nu, size_t Nv);
mat3 rebond(vec3& normale);
mesh_drawable_hierarchy create_man();
float dist(vec3& a, vec3& b);
vcl::mesh create_skybox();


/** This function is called before the beginning of the animation loop
    It is used to initialize all part-specific data */
void scene_exercise::setup_data(std::map<std::string,GLuint>& , scene_structure& scene, gui_structure& )
{
    /*
     * TERRAIN
     */
    terrain.data_gpu.clear(); // Clear memory in case of pre-existing terrain
    terrain = create_terrain(); // Create visual terrain surface
    terrain.uniform_parameter.shading.specular = 0.0f; // non-specular terrain material
    
    /*
     * BASKETBALL STANDS
     */
    stand[0] = create_stand();
    stand[1] = create_stand();

    /*
     * GRASS
     */
    // Create a quad with (u,v)-texture coordinates
    mesh grass_cpu;
    grass_cpu.position     = {{-0.3f, 0, 0}, {0.3f, 0, 0}, {0.3f, 0.6f, 0}, {-0.3f, 0.6f, 0}};
    grass_cpu.texture_uv   = {{0,1}, {1,1}, {1,0}, {0,0}};
    grass_cpu.connectivity = {{0,1,2}, {0,2,3}};

    for (int i=0; i<num_grass; i++)
    {
        grass[i].data_gpu.clear();
        grass[i] = grass_cpu;
        grass[i].uniform_parameter.shading = {1,0,0}; // set pure ambiant component (no diffuse, no specular) - allow to only see the color of the texture
        float x = distrib(generator);
        float y = distrib(generator);
        grass[i].uniform_parameter.translation = evaluate_terrain(x, y);
    }

    // Load basketball court texture
    texture_terrain_id = texture_gpu(image_load_png("data/basketball_court.png"));

    // Load grass texture (with transparent background)
    texture_grass_id = texture_gpu(image_load_png("data/cactus.png"));

    // Load basketball board texture
    texture_board_id = texture_gpu(image_load_png("data/basketball_board.png"));

    // Setup initial camera mode and position
    scene.camera.camera_type = camera_control_spherical_coordinates;
    scene.camera.apply_translation_orthogonal_to_screen_plane(-1.5f);
    scene.camera.scale = 5.0f;
    scene.camera.apply_rotation(-M_PI/2,-M_PI/2,0,0);

    // Initial position and speed of particles
    // ******************************************* //

    for (int i=0; i<num_string; i++)
    {
        float theta = 2*M_PI/num_string;
        float c = ring_radius*std::cos(theta*i);
        float s = ring_radius*std::sin(theta*i);

        pA[i].p = ring_position1 + vec3(c, s, 0);
        pA[i].v = {0,0,0};

        pB[i].p = ring_position1 + vec3(c, s, 0);
        pB[i].v = {0,0,0};

        pC[i].p = ring_position1 + vec3(c, s, 0);
        pC[i].v = {0,0,0};

        pD[i].p = ring_position1 + vec3(c, s, 0);
        pD[i].v = {0,0,0};

        pA[num_string+i].p = ring_position2 + vec3(c, s, 0);
        pA[num_string+i].v = {0,0,0};

        pB[num_string+i].p = ring_position2 + vec3(c, s, 0);
        pB[num_string+i].v = {0,0,0};

        pC[num_string+i].p = ring_position2 + vec3(c, s, 0);
        pC[num_string+i].v = {0,0,0};

        pD[num_string+i].p = ring_position2 + vec3(c, s, 0);
        pD[num_string+i].v = {0,0,0};
    }
    
    // Display elements
    // ******************************************* //
    segment_drawer.init();
    segment_drawer.uniform_parameter.color = {0,0,1};

    sphere = mesh_primitive_sphere(0.2f);
    sphere.uniform_parameter.scaling = 0.05f;

    man = create_man();

    ball = mesh_primitive_sphere(ball_radius);
    ball.uniform_parameter.color = {1.0f, 0.35f, 0.0f};

    period.periodic_event_time_step = 1.0f;

    // Information about basketball stands (collision)
    // ******************************************* //
    ring1 = mesh_primitive_ring(ring_radius, ring_radius/10.0f, ring_position1, {0,0,1}, 50, 30);
    ring2 = mesh_primitive_ring(ring_radius, ring_radius/10.0f, ring_position2, {0,0,1}, 50, 30);
    pillar1 = mesh_primitive_cylinder(stand_radius, stand_position1, stand_position1 + vec3(0, 0, stand_height), 20, 20);
    pillar2 = mesh_primitive_cylinder(stand_radius, stand_position2, stand_position2 + vec3(0, 0, stand_height), 20, 20);
    join_ball1_1 = mesh_primitive_sphere(stand_radius, join_ball1_position1, 20, 20);
    join_ball2_1 = mesh_primitive_sphere(stand_radius, join_ball2_position1, 20, 20);
    join_ball1_2 = mesh_primitive_sphere(stand_radius, join_ball1_position2, 20, 20);
    join_ball2_2 = mesh_primitive_sphere(stand_radius, join_ball2_position2, 20, 20);
    connection1_1 = mesh_primitive_cylinder(stand_radius, join_ball1_position1, join_ball2_position1, 20, 20);
    connection2_1 = mesh_primitive_cylinder(stand_radius, join_ball2_position1, board_position1, 20, 20);
    connection1_2 = mesh_primitive_cylinder(stand_radius, join_ball1_position2, join_ball2_position2, 20, 20);
    connection2_2 = mesh_primitive_cylinder(stand_radius, join_ball2_position2, board_position2, 20, 20);
    
    // Picked object
    // ******************************************* //
    picked_object=-1;

    // Skybox
    // ******************************************* //
    skybox = create_skybox();
    skybox.uniform_parameter.shading = {1,0,0};
    skybox.uniform_parameter.rotation = rotation_from_axis_angle_mat3({1,0,0},-3.014f/2.0f);
    texture_skybox = texture_gpu(image_load_png("data/desertpreview.png"));

}


/** This function is called at each frame of the animation loop.
    It is used to compute time-varying argument and perform data data drawing */
void scene_exercise::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& )
{
    // Before displaying a textured surface: bind the associated texture id
    glBindTexture(GL_TEXTURE_2D, texture_terrain_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // u
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // v

    // Display terrain
    glPolygonOffset(1.0, 1.0);
    terrain.draw(shaders["mesh"], scene.camera);

    glBindTexture(GL_TEXTURE_2D, scene.texture_white);

    glBindTexture(GL_TEXTURE_2D, texture_board_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // u
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // v

    // Display stands
    display_stands(shaders,scene);

    // Display basketball net

    display_net(shaders,scene);

    display_man(shaders,scene);
    display_ball(shaders,scene);
    display_skybox(shaders,scene);


    // Enable use of alpha component as color blending for transparent elements
    // new color = previous color + (1-alpha) current color
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth buffer writing
    //  - Transparent elements cannot use depth buffer
    //  - They are supposed to be display from furest to nearest elements
    glDepthMask(false);

    glEnable(GL_POLYGON_OFFSET_FILL); // avoids z-fighting when displaying wireframe

    glBindTexture(GL_TEXTURE_2D, texture_grass_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // avoids sampling artifacts
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // avoids sampling artifacts
    

    // Display a billboard always facing the camera direction
    for (int i=0; i<num_grass; i++)
    {
        grass[i].uniform_parameter.rotation = scene.camera.orientation;
        grass[i].draw(shaders["mesh"], scene.camera);
        if(gui_scene.wireframe)
            grass[i].draw(shaders["wireframe"], scene.camera);
    }

    glBindTexture(GL_TEXTURE_2D, scene.texture_white);

    glDepthMask(true);
}

void scene_exercise::display_stands(std::map<std::string,GLuint>& shaders, scene_structure& scene){

    stand[0].translation("pillar") = stand_position1;
    stand[0].draw(shaders["mesh"], scene.camera);
    stand[1].translation("pillar") = stand_position2;
    stand[1].rotation("pillar") = rotation_from_axis_angle_mat3({0, 0, 1}, M_PI);
    stand[1].draw(shaders["mesh"], scene.camera);
    if(gui_scene.wireframe)
    {
        stand[0].draw(shaders["wireframe"], scene.camera);
        stand[1].draw(shaders["wireframe"], scene.camera);
    }

    glBindTexture(GL_TEXTURE_2D, scene.texture_white);
}

void scene_exercise::display_net(std::map<std::string,GLuint>& shaders, scene_structure& scene){

    timer.update();
    set_gui(scene);
    float dt = timer.scale*0.01f;

    // 2 basketball nets: left and right
    for (int j=0; j<2; j++)
    {
        for (int i=0; i<num_string; i++)
        {
            // Vertical forces
            const vec3 fAB_spring = spring_force(pB[j*num_string+i].p, pA[j*num_string+i].p, L_AB, K);
            const vec3 fBC_spring = spring_force(pC[j*num_string+i].p, pB[j*num_string+i].p, L_BC, K);
            const vec3 fCD_spring = spring_force(pD[j*num_string+i].p, pC[j*num_string+i].p, L_CD, K);

            // Horizontal forces
            vec3 fB1_spring;
            vec3 fC1_spring;
            vec3 fD1_spring;
            vec3 fB2_spring;
            vec3 fC2_spring;
            vec3 fD2_spring;

            if (i < num_string-1 && i > 0)
            {
                fB1_spring = spring_force(pB[j*num_string+i].p, pB[j*num_string+i+1].p, L_horizontal_B, K);
                fC1_spring = spring_force(pC[j*num_string+i].p, pC[j*num_string+i+1].p, L_horizontal_C, K);
                fD1_spring = spring_force(pD[j*num_string+i].p, pD[j*num_string+i+1].p, L_horizontal_D, K);
                fB2_spring = spring_force(pB[j*num_string+i].p, pB[j*num_string+i-1].p, L_horizontal_B, K);
                fC2_spring = spring_force(pC[j*num_string+i].p, pC[j*num_string+i-1].p, L_horizontal_C, K);
                fD2_spring = spring_force(pD[j*num_string+i].p, pD[j*num_string+i-1].p, L_horizontal_D, K);
            }
            else if (i == 0)
            {
                fB1_spring = spring_force(pB[j*num_string+i].p, pB[j*num_string+i+1].p, L_horizontal_B, K);
                fC1_spring = spring_force(pC[j*num_string+i].p, pC[j*num_string+i+1].p, L_horizontal_C, K);
                fD1_spring = spring_force(pD[j*num_string+i].p, pD[j*num_string+i+1].p, L_horizontal_D, K);
                fB2_spring = spring_force(pB[j*num_string+i].p, pB[j*num_string+num_string-1].p, L_horizontal_B, K);
                fC2_spring = spring_force(pC[j*num_string+i].p, pC[j*num_string+num_string-1].p, L_horizontal_C, K);
                fD2_spring = spring_force(pD[j*num_string+i].p, pD[j*num_string+num_string-1].p, L_horizontal_D, K);
            }
            else
            {
                fB1_spring = spring_force(pB[j*num_string+i].p, pB[j*num_string].p, L_horizontal_B, K);
                fC1_spring = spring_force(pC[j*num_string+i].p, pC[j*num_string].p, L_horizontal_C, K);
                fD1_spring = spring_force(pD[j*num_string+i].p, pD[j*num_string].p, L_horizontal_D, K);
                fB2_spring = spring_force(pB[j*num_string+i].p, pB[j*num_string+i-1].p, L_horizontal_B, K);
                fC2_spring = spring_force(pC[j*num_string+i].p, pC[j*num_string+i-1].p, L_horizontal_C, K);
                fD2_spring = spring_force(pD[j*num_string+i].p, pD[j*num_string+i-1].p, L_horizontal_D, K);
            }

            const vec3 fB_damping = -mu_dampling*pB[j*num_string+i].v;
            const vec3 fC_damping = -mu_dampling*pC[j*num_string+i].v;
            const vec3 fD_damping = -mu_dampling*pD[j*num_string+i].v;
            const vec3 FB = fB1_spring + fB2_spring - fBC_spring + fAB_spring + f_weight + fB_damping;
            const vec3 FC = fC1_spring + fC2_spring - fCD_spring + fBC_spring + f_weight + fC_damping;
            const vec3 FD = fD1_spring + fD2_spring + fCD_spring + f_weight + fD_damping;

            // Numerical Integration (Verlet)
            {
                pB[j*num_string+i].v = pB[j*num_string+i].v + dt*FB/m;
                pB[j*num_string+i].p = pB[j*num_string+i].p + dt*pB[j*num_string+i].v;
                pC[j*num_string+i].v = pC[j*num_string+i].v + dt*FC/m;
                pC[j*num_string+i].p = pC[j*num_string+i].p + dt*pC[j*num_string+i].v;
                pD[j*num_string+i].v = pD[j*num_string+i].v + dt*FD/m;
                pD[j*num_string+i].p = pD[j*num_string+i].p + dt*pD[j*num_string+i].v;
            }
            
            for (particle_element& p_ball : ball_particles)
            {
                if (dist(pB[j*num_string+i].p, p_ball.p) < ball_radius){
                    pB[j*num_string+i].v = p_ball.v;
                }

                if (dist(pC[j*num_string+i].p, p_ball.p) < ball_radius){
                    pC[j*num_string+i].v = p_ball.v;
                }

                if (dist(pD[j*num_string+i].p, p_ball.p) < ball_radius){
                    pD[j*num_string+i].v = p_ball.v;
                }
            }
            // Display of the result

            // particle pa
            sphere.uniform_parameter.translation = pA[j*num_string+i].p;
            sphere.uniform_parameter.color = {0,0,1};
            sphere.draw(shaders["mesh"], scene.camera);

            // particle pb
            sphere.uniform_parameter.translation = pB[j*num_string+i].p;
            sphere.uniform_parameter.color = {0,0,1};
            sphere.draw(shaders["mesh"], scene.camera);

            // particle pc
            sphere.uniform_parameter.translation = pC[j*num_string+i].p;
            sphere.uniform_parameter.color = {0,0,1};
            sphere.draw(shaders["mesh"], scene.camera);

            // particle pd
            sphere.uniform_parameter.translation = pD[j*num_string+i].p;
            sphere.uniform_parameter.color = {0,0,1};
            sphere.draw(shaders["mesh"], scene.camera);

            // Spring pa-pb
            segment_drawer.uniform_parameter.p1 = pA[j*num_string+i].p;
            segment_drawer.uniform_parameter.p2 = pB[j*num_string+i].p;
            segment_drawer.draw(shaders["segment_im"],scene.camera);

            // Spring pb-pc
            segment_drawer.uniform_parameter.p1 = pB[j*num_string+i].p;
            segment_drawer.uniform_parameter.p2 = pC[j*num_string+i].p;
            segment_drawer.draw(shaders["segment_im"],scene.camera);

            // Spring pc-pd
            segment_drawer.uniform_parameter.p1 = pC[j*num_string+i].p;
            segment_drawer.uniform_parameter.p2 = pD[j*num_string+i].p;
            segment_drawer.draw(shaders["segment_im"],scene.camera);

            if (i < num_string-1)
            {
                // Spring a[j*num_string+i]-a[j*num_string+i+1]
                segment_drawer.uniform_parameter.p1 = pA[j*num_string+i].p;
                segment_drawer.uniform_parameter.p2 = pA[j*num_string+i+1].p;
                segment_drawer.draw(shaders["segment_im"],scene.camera);

                // Spring b[j*num_string+i]-b[j*num_string+i+1]
                segment_drawer.uniform_parameter.p1 = pB[j*num_string+i].p;
                segment_drawer.uniform_parameter.p2 = pB[j*num_string+i+1].p;
                segment_drawer.draw(shaders["segment_im"],scene.camera);

                // Spring c[j*num_string+i]-c[j*num_string+i+1]
                segment_drawer.uniform_parameter.p1 = pC[j*num_string+i].p;
                segment_drawer.uniform_parameter.p2 = pC[j*num_string+i+1].p;
                segment_drawer.draw(shaders["segment_im"],scene.camera);

                // Spring d[j*num_string+i]-d[j*num_string+i+1]
                segment_drawer.uniform_parameter.p1 = pD[j*num_string+i].p;
                segment_drawer.uniform_parameter.p2 = pD[j*num_string+i+1].p;
                segment_drawer.draw(shaders["segment_im"],scene.camera);
            }

            // Spring a[j*num_string+i]-a[j*num_string+i+1]
            segment_drawer.uniform_parameter.p1 = pA[j*num_string+num_string-1].p;
            segment_drawer.uniform_parameter.p2 = pA[j*num_string].p;
            segment_drawer.draw(shaders["segment_im"],scene.camera);

            // Spring b[j*num_string+i]-b[j*num_string+i+1]
            segment_drawer.uniform_parameter.p1 = pB[j*num_string+num_string-1].p;
            segment_drawer.uniform_parameter.p2 = pB[j*num_string].p;
            segment_drawer.draw(shaders["segment_im"],scene.camera);

            // Spring c[j*num_string+i]-c[j*num_string+i+1]
            segment_drawer.uniform_parameter.p1 = pC[j*num_string+num_string-1].p;
            segment_drawer.uniform_parameter.p2 = pC[j*num_string].p;
            segment_drawer.draw(shaders["segment_im"],scene.camera);

            // Spring d[j*num_string+i]-d[j*num_string+i+1]
            segment_drawer.uniform_parameter.p1 = pD[j*num_string+num_string-1].p;
            segment_drawer.uniform_parameter.p2 = pD[j*num_string].p;
            segment_drawer.draw(shaders["segment_im"],scene.camera);
        }
    }
}

// Compute spring force applied on particle pi from particle pj: F_spring=−K(L−L0)u
vec3 spring_force(const vec3& pi, const vec3& pj, float L0, float K)
{
    float L = norm(pj-pi);
    vec3 u = normalize(pi-pj);
    return -K*(L-L0)*u;
}

// Evaluate 3D position of the terrain for any (u,v) \in [0,1]
vec3 evaluate_terrain(float u, float v)
{
    // texture: 18.78*11.67
    const float x = 20.0f*(u-0.5f);
    const float y = 12.5f*(v-0.5f);
    const float z = 0.0f;

    return {x,y,z};
}

// Generate terrain mesh
mesh create_terrain()
{
    // Number of samples of the terrain is N x N
    const size_t N = 100;

    mesh terrain; // temporary terrain storage (CPU only)
    terrain.position.resize(N*N);
    terrain.texture_uv.resize(N*N);

    // Fill terrain geometry
    for(size_t ku=0; ku<N; ++ku)
    {
        for(size_t kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(N-1.0f);
            const float v = kv/(N-1.0f);

            // Compute coordinates
            terrain.position[kv+N*ku] = evaluate_terrain(u, v);

            const float ru = 18.78f/20.0f;
            const float rv = 11.67f/12.5f;
            
            terrain.texture_uv[kv+N*ku] = {(u-0.5f)/ru+0.5f, (v-0.5f)/rv+0.5f};
        }
    }


    // Generate triangle organization
    // Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    const unsigned int Ns = N;
    for(unsigned int ku=0; ku<Ns-1; ++ku)
    {
        for(unsigned int kv=0; kv<Ns-1; ++kv)
        {
            const unsigned int idx = kv + N*ku; // current vertex offset

            const index3 triangle_1 = {idx, idx+1+Ns, idx+1};
            const index3 triangle_2 = {idx, idx+Ns, idx+1+Ns};

            terrain.connectivity.push_back(triangle_1);
            terrain.connectivity.push_back(triangle_2);
        }
    }

    return terrain;
}

mesh_drawable_hierarchy scene_exercise::create_stand()
{
    mesh_drawable_hierarchy stand;

    mesh_drawable pillar = mesh_primitive_cylinder(stand_radius, {0, 0, 0}, {0, 0, stand_height}, 20, 20);
    pillar.uniform_parameter.color = {1.0f, 1.0f, 1.0f};
    pillar.uniform_parameter.shading.specular = 0.0f;

    mesh_drawable join_ball = mesh_primitive_sphere(stand_radius, {0,0,0}, 20, 20);
    join_ball.uniform_parameter.color = {1.0f, 1.0f, 1.0f};
    join_ball.uniform_parameter.shading.specular = 0.0f;
    
    mesh_drawable connection1 = mesh_primitive_cylinder(stand_radius, {0, 0, 0}, {0, 0, stand_height/3.0f}, 20, 20);
    connection1.uniform_parameter.color = {1.0f, 1.0f, 1.0f};
    connection1.uniform_parameter.shading.specular = 0.0f;
    
    mesh_drawable connection2 = mesh_primitive_cylinder(stand_radius, {0, 0, 0}, {0, 0, stand_height/5.0f}, 20, 20);
    connection2.uniform_parameter.color = {1.0f, 1.0f, 1.0f};
    connection2.uniform_parameter.shading.specular = 0.0f;
    
    // Board relative position
    const vcl::vec3 b01 = {-board_width*2.0f/3.0f, board_length/2.0f, 0.0f};
    const vcl::vec3 b11 = {board_width/3.0f, board_length/2.0f, 0.0f};
    const vcl::vec3 b10 = {board_width/3.0f, -board_length/2.0f, 0.0f};
    const vcl::vec3 b00 = {-board_width*2.0f/3.0f, -board_length/2.0f, 0.0f};

    mesh board = mesh_primitive_quad(b00, b10, b11, b01);
    board.texture_uv = {{0,0}, {0,1}, {1,1}, {1,0}};
    
    // Connection (ring-board) relative position
    const vcl::vec3 c01 = {ring_board_gap, -connection_length/2.0f, 0.0f};
    const vcl::vec3 c11 = {ring_board_gap, connection_length/2.0f, 0.0f};
    const vcl::vec3 c10 = {ring_board_gap, connection_length/2.0f, connection_width};
    const vcl::vec3 c00 = {ring_board_gap, -connection_length/2.0f, connection_width};

    mesh_drawable connection3 = mesh_primitive_quad(c00, c10, c11, c01);
    connection3.uniform_parameter.color = {1.0f, 0.0f, 0.0f};
    connection3.uniform_parameter.shading.specular = 0.0f;
    
    mesh_drawable ring = mesh_primitive_ring(ring_radius, ring_radius/10.0f, {0,0,0}, {1,0,0}, 50, 40);


    ring.uniform_parameter.color = {1.0f, 0.0f, 0.0f};
    ring.uniform_parameter.shading.specular = 0.0f;

    stand.add_element(pillar, "pillar", "root");
    stand.add_element(join_ball, "join_ball1", "pillar", {0.0f, 0.0f, stand_height});
    stand.add_element(connection1, "connection1", "join_ball1", {0.0f, 0.0f, 0.0f}, rotation_from_axis_angle_mat3({0, 1, 0}, M_PI/6.0f));
    stand.add_element(join_ball, "join_ball2", "connection1", {0.0f, 0.0f, stand_height/3.0f});
    stand.add_element(connection2, "connection2", "join_ball2", {0.0f, 0.0f, 0.0f}, rotation_from_axis_angle_mat3({0, 1, 0}, M_PI/3.0f));
    stand.add_element(board, "board", "connection2", {0.0f, 0.0f, stand_height*1.05f/5.0f});
    stand.add_element(connection3, "connection3", "board");
    stand.add_element(ring, "ring", "connection3", {board_width/6.0f, 0, ring_radius+board_width/10.0f});
    return stand;
}

void scene_exercise::display_man(std::map<std::string,GLuint>& shaders, scene_structure& scene)
{

    const float t = timer.t-timesincenewball;


    man.rotation("body") = rotation_from_axis_angle_mat3({0,0,1}, M_PI/2.0f+ gui_scene.man_phi*M_PI/180.f);
    float rot=0.0f;

    if(t<1.0f){
        rot=t;
    }
    const float thetaleftarm2 = std::cos(2*M_PI*rot-M_PI)/2.0f - M_PI/8.0f;
    man.rotation("leftarm2") = rotation_from_axis_angle_mat3({0,0,1}, thetaleftarm2);
    const float thetarightarm2 = std::cos(2*M_PI*rot)/2.0f + M_PI/8.0f;
    man.rotation("rightarm2") = rotation_from_axis_angle_mat3({0,0,1}, thetarightarm2);

    const float thetaleftarm = std::cos(2*M_PI*rot)/5-0.6f;
    const float thetarightarm = std::cos(2*M_PI*rot+ M_PI)/5+0.6f;

    man.rotation("leftarm") = rotation_from_axis_angle_mat3({0,2*std::sin(gui_scene.ball_theta*3.14f/180.0f),2*std::cos(gui_scene.ball_theta*3.14f/180.0f)}, thetaleftarm);
    man.rotation("rightarm") = rotation_from_axis_angle_mat3({0,2*std::sin(gui_scene.ball_theta*3.14f/180.0f),2*std::cos(gui_scene.ball_theta*3.14f/180.0f)}, thetarightarm);

    man.translation("body") = {gui_scene.man_x, gui_scene.man_y, 0.925f};


    man.draw(shaders["mesh"], scene.camera);
}

void scene_exercise::display_ball(std::map<std::string,GLuint>& shaders, scene_structure& scene)
{

    const float dt = period.update()/10;

    if (timer.t-timesincenewball>1){
        mesh_drawable ballhand=mesh_primitive_sphere(ball_radius);
        vec3 i = {0.3f*std::cos(gui_scene.man_phi*3.141592f/180.0f), 0.3f*std::sin(gui_scene.man_phi*3.141592f/180.0f),1.2f};
        vec3 p0 = i + vec3{gui_scene.man_x,gui_scene.man_y,0.0f};
        ballhand.uniform_parameter.translation=p0;
        ballhand.uniform_parameter.color={1.0f, 0.35f, 0.0f};
        ballhand.draw(shaders["mesh"], scene.camera);
    }

    if (newball)
    {
        particle_element new_particle;
        vec3 i = {0.3f*std::cos(gui_scene.man_phi*3.141592f/180.0f), 0.3f*std::sin(gui_scene.man_phi*3.141592f/180.0f),1.2f};
        vec3 p0 = i + vec3{gui_scene.man_x,gui_scene.man_y,0.0f};
        vec3 v0 = vec3( gui_scene.ball_speed*std::cos(gui_scene.ball_theta*3.141592f/180.0f)*std::cos(gui_scene.man_phi*3.141592f/180.0f), gui_scene.ball_speed*std::cos(gui_scene.ball_theta*3.141592f/180.0f)*std::sin(gui_scene.man_phi*3.141592f/180.0f), gui_scene.ball_speed*std::sin(gui_scene.ball_theta*3.141592f/180.0f));
        ball_particles.push_back({p0, v0, false, false});
        newball=false;
        timesincenewball=timer.t;
    }

    const vec3 g = {0.0f, 0.0f, -gravity};
    for (particle_element& p_ball : ball_particles)
    {
        for (int j=0; j<10; j++)
        {
            const float m = 0.01f;
            const vec3 F = m*g;
            vec3& p = p_ball.p;
            vec3& v = p_ball.v;

            v = v + dt*F/m - mu_air*v;
            p = p + dt*v;

            // Collision stuffs
            // ******************************************* //

            // bounce on the ground
            if (p[2] < ball_radius && p[0] < 10.0f && p[0] > -10.0f && p[1] < 6.25f && p[1] > -6.25f && v[2]*p[2]<0){
                v[2]*=-rebound_coefficient;
                p[2]=ball_radius;
            }
            // bounce on the left board
            if (p[0] > board_position2[0]-ball_radius && p[0] < board_position2[0]+ball_radius && p[1] < board_length/2.0f+ball_radius && p[1] > -board_length/2.0f-ball_radius && p[2] < board_position2[2]+board_width*2.0f/3.0f+ball_radius && p[2] > board_position2[2]-board_width/3.0f-ball_radius && v[0]>0)
                v[0]*=-rebound_coefficient;

            // bounce on the right board
            if (p[0] > board_position1[0]-ball_radius && p[0] < board_position1[0]+ball_radius && p[1] < board_length/2.0f+ball_radius && p[1] > -board_length/2.0f-ball_radius && p[2] < board_position1[2]+board_width*2.0f/3.0f+ball_radius && p[2] > board_position1[2]-board_width/3.0f-ball_radius && v[0]<0)
                v[0]*=-rebound_coefficient;

            // bounce on the left ring-board connection
            if (p[0] > board_position2[0]-connection_width-ball_radius && p[0] < board_position2[0]+ball_radius && p[1] < connection_length/2.0f+ball_radius && p[1] > -connection_length/2.0f-ball_radius && p[2] < board_position2[2]-ring_board_gap+ball_radius && p[2] > board_position2[2]-ring_board_gap-ball_radius)
                v[2]*=-rebound_coefficient;

            // bounce on the right ring-board connection 
            if (p[0] > board_position1[0]+connection_width-ball_radius && p[0] < board_position1[0]+ball_radius && p[1] < connection_length/2.0f+ball_radius && p[1] > -connection_length/2.0f-ball_radius && p[2] < board_position2[2]-ring_board_gap+ball_radius && p[2] > board_position2[2]-ring_board_gap-ball_radius)
                v[2]*=-rebound_coefficient;

            unsigned int i;
            bool test;
            bool previous_collision_ring = p_ball.previous_collision_ring;
            bool previous_collision_stand = p_ball.previous_collision_stand;

            // bounce rings
            i = 0;
            test = false;

            while (!test && i < ring1.position.size()){
                if (dist(p, ring1.position[i]) < ball_radius){
                    test = true;
                    if (!previous_collision_ring){
                        v = rebound_coefficient*rebond(ring1.normal[i])*v;
                        previous_collision_ring = true;
                    }
                }
                if (dist(p, ring2.position[i]) < ball_radius){
                    test = true;
                    if (!previous_collision_ring){
                        v = rebound_coefficient*rebond(ring2.normal[i])*v;
                        previous_collision_ring = true;
                    }
                }
                i++;
                if (i == ring1.position.size())
                    previous_collision_ring = false;
            }
            p_ball.previous_collision_ring = previous_collision_ring;

            // bounce basketball stands
            i = 0;
            test = false;

            while (!test && i < pillar1.position.size()){
                if (dist(p, pillar1.position[i]) < ball_radius){
                    test = true;
                    if (!previous_collision_stand){
                        v = rebound_coefficient*rebond(pillar1.normal[i])*v;
                        previous_collision_stand = true;
                    }
                }
                if (dist(p, pillar2.position[i]) < ball_radius){
                    test = true;    
                    if (!previous_collision_stand){
                        v = rebound_coefficient*rebond(pillar2.normal[i])*v;
                        previous_collision_stand = true;
                    }
                }                
                if (dist(p, connection1_1.position[i]) < ball_radius){
                    test = true;
                    if (!previous_collision_stand){
                        v = rebound_coefficient*rebond(connection1_1.normal[i])*v;
                        previous_collision_stand = true;
                    }
                }
                if (dist(p, connection1_2.position[i]) < ball_radius){
                    test = true;    
                    if (!previous_collision_stand){
                        v = rebound_coefficient*rebond(connection1_2.normal[i])*v;
                        previous_collision_stand = true;
                    }
                }
                if (dist(p, connection2_1.position[i]) < ball_radius){
                    test = true;
                    if (!previous_collision_stand){
                        v = rebound_coefficient*rebond(connection2_1.normal[i])*v;
                        previous_collision_stand = true;
                    }
                }
                if (dist(p, connection2_2.position[i]) < ball_radius){
                    test = true;
                    if (!previous_collision_stand){
                        v = rebound_coefficient*rebond(connection2_2.normal[i])*v;
                        previous_collision_stand = true;
                    }
                }
                if (dist(p, join_ball1_1.position[i]) < ball_radius){
                    test = true;
                    if (!previous_collision_stand){
                        v = rebound_coefficient*rebond(join_ball1_1.normal[i])*v;
                        previous_collision_stand = true;
                    }
                }
                if (dist(p, join_ball1_2.position[i]) < ball_radius){
                    test = true;
                    if (!previous_collision_stand){
                        v = rebound_coefficient*rebond(join_ball1_2.normal[i])*v;
                        previous_collision_stand = true;
                    }
                }
                if (dist(p, join_ball2_1.position[i]) < ball_radius){
                    test = true;
                    if (!previous_collision_stand){
                        v = rebound_coefficient*rebond(join_ball2_1.normal[i])*v;
                        previous_collision_stand = true;
                    }
                }
                if (dist(p, join_ball2_2.position[i]) < ball_radius){
                    test = true;
                    if (!previous_collision_stand){
                        v = rebound_coefficient*rebond(join_ball2_2.normal[i])*v;
                        previous_collision_stand = true;
                    }
                }
                i++;
                if (i == pillar1.position.size())
                    previous_collision_stand = false;
            }
            p_ball.previous_collision_stand = previous_collision_stand;
        }
    }

    if (ball_particles.size() > 5)
        ball_particles.begin() = ball_particles.erase(ball_particles.begin());

    for(auto it = ball_particles.begin(); it!=ball_particles.end(); ++it)
        if( it->p.z < -3)
            it = ball_particles.erase(it);

    for (particle_element& p_ball : ball_particles)
    {
        ball.uniform_parameter.translation = p_ball.p;
        ball.draw(shaders["mesh"], scene.camera);
    }


}

mat3 rebond (vec3& n){
    vec3 normale = normalize(n);
    return mat3(1-2*normale.x*normale.x,-2*normale.x*normale.y,-2*normale.x*normale.z,-2*normale.x*normale.y,1-2*normale.y*normale.y,-2*normale.y*normale.z,-2*normale.x*normale.z,-2*normale.y*normale.z,1-2*normale.z*normale.z);
}

float dist(vec3& a,vec3& b){
    float d2 = (b[0]-a[0])*(b[0]-a[0])+(b[1]-a[1])*(b[1]-a[1])+(b[2]-a[2])*(b[2]-a[2]);
    return std::sqrt(d2);
}

mesh_drawable_hierarchy create_man()
{
    mesh_drawable_hierarchy hierarchy;
    mesh body = mesh_primitive_sphere(1.0f);
    for(size_t k=0; k<body.position.size(); ++k)
    {
        body.position[k].x *= 0.45f;
        body.position[k].y *= 0.3f;
    }
    hierarchy.add_element(body, "body","root", {0,0,0});
    hierarchy.mesh_visual("body").uniform_parameter.color = {0.0f,0.0f,1.0f};

    mesh head = mesh_primitive_sphere(0.4f);
    hierarchy.add_element(head,"head","body", {0.0,0.0,1.1});

    mesh leftarm = mesh_primitive_cylinder(0.1f, {0.0f,0.0f,0.0f}, {0.7f,0.0f,0.0f}, 15, 15);
    hierarchy.add_element(leftarm,"leftarm","body", {0.3f,0.0f,0.6f});

    mesh rightarm = mesh_primitive_cylinder(0.1f, {0.0f,0.0f,0.0f}, {-0.7f,0.0f,0.0f}, 15, 15);
    hierarchy.add_element(rightarm,"rightarm","body", {-0.3f,0.0f,0.6f});

    mesh leftleg = mesh_primitive_cylinder(0.1f, {0.0f,0.0f,0.0f}, {0.2f,0.0f,-0.6f}, 15, 15);
    hierarchy.add_element(leftleg,"leftleg","body", {0.1f,0.0f,-0.9f});
    hierarchy.mesh_visual("leftleg").uniform_parameter.color = {1.0f,0.8f,0.0f};

    mesh rightleg = mesh_primitive_cylinder(0.1f, {0.0f,0.0f,0.0f}, {-0.2f,0.0f,-0.6f}, 15, 15);
    hierarchy.add_element(rightleg,"rightleg","body", {-0.1f,0.0f,-0.9f});
    hierarchy.mesh_visual("rightleg").uniform_parameter.color = {1.0f,0.8f,0.0f};


    mesh leftknee = mesh_primitive_sphere(0.1f);
    hierarchy.add_element(leftknee,"leftknee","leftleg", {0.2f,0.0f,-0.6f});

    mesh rightknee = mesh_primitive_sphere(0.1f);
    hierarchy.add_element(rightknee,"rightknee","rightleg", {-0.2f,0.0f,-0.6f});

    mesh leftleg2 = mesh_primitive_cylinder(0.1f, {0.0f,0.0f,0.0f}, {0.0f,0.0f,-0.6f}, 15, 15);
    hierarchy.add_element(leftleg2,"leftleg2","leftknee", {0.0f,0.0f,-0.6f});

    mesh rightleg2 = mesh_primitive_cylinder(0.1f, {0.0f,0.0f,0.0f}, {0.0f,0.0f,-0.6f}, 15, 15);
    hierarchy.add_element(rightleg2,"rightleg2","rightknee", {-0.0f,0.0f,-0.6f});

    mesh leftfoot = mesh_primitive_parallelepiped({0.0f,0.0f,0.0f},{0.2f,0.0f,0.0f},{0.0f,-0.5f,0.0f},{0.0f,0.0f,-0.2f});
    hierarchy.add_element(leftfoot,"leftfoot","leftleg2", {-0.1f,0.1f,0.0f});
    hierarchy.mesh_visual("leftfoot").uniform_parameter.color = {0.5f,0.5f,0.5f};

    mesh rightfoot = mesh_primitive_parallelepiped({0.0f,0.0f,0.0f},{0.2f,0.0f,0.0f},{0.0f,-0.5f,0.0f},{0.0f,0.0f,-0.2f});
    hierarchy.add_element(rightfoot,"rightfoot","rightleg2", {-0.1f,0.1f,-0.0f});
    hierarchy.mesh_visual("rightfoot").uniform_parameter.color = {0.5f,0.5f,0.5f};



    mesh leftelbow = mesh_primitive_sphere(0.1f);
    hierarchy.add_element(leftelbow,"leftelbow","leftarm", {0.7f,0.0f,0.0f});

    mesh rightelbow = mesh_primitive_sphere(0.1f);
    hierarchy.add_element(rightelbow,"rightelbow","rightarm", {-0.7f,0.0f,0.0f});

    mesh leftarm2 = mesh_primitive_cylinder(0.1f, {0.0f,0.0f,0.0f}, {0.0f,-0.7f,0.0f}, 15, 15);
    hierarchy.add_element(leftarm2,"leftarm2","leftelbow", {0.0f,0.0f,0.0f});

    mesh rightarm2 = mesh_primitive_cylinder(0.1f, {0.0f,0.0f,0.0f}, {0.0f,-0.7f,0.0f}, 15, 15);
    hierarchy.add_element(rightarm2,"rightarm2","rightelbow", {-0.0f,0.0f,0.0f});

    mesh lefthand = mesh_primitive_parallelepiped({0.0f,0.0f,0.0f},{0.2f,0.0f,0.0f},{0.0f,0.05f,0.0f},{0.0f,0.0f,0.4f});
    hierarchy.add_element(lefthand,"lefthand","leftarm2", {-0.1f,-0.7f,-0.1f});

    mesh righthand = mesh_primitive_parallelepiped({0.0f,0.0f,0.0f},{0.2f,0.0f,0.0f},{0.0f,0.05f,0.0f},{0.0f,0.0f,0.4f});
    hierarchy.add_element(righthand,"righthand","rightarm2", {-0.1f,-0.7f,-0.1f});

    hierarchy.scaling=0.4f;

    return hierarchy;
}


void scene_exercise::set_gui(scene_structure& scene)
{
    ImGui::Checkbox("Wireframe", &gui_scene.wireframe);


    if (ImGui::Button("Newball (key Enter)"))
        newball=true;
    float x = gui_scene.man_x;
    float y = gui_scene.man_y;
    float phi= gui_scene.man_phi;

    ImGui::SliderFloat("Ball speed (key C & V)", &gui_scene.ball_speed, 5.0f, 15.0f);
    ImGui::SliderFloat("Ball theta (key E & D)", &gui_scene.ball_theta, -90.0f, 90.0f);
    ImGui::SliderFloat("Man phi (arrow keys)", &gui_scene.man_phi, -180.0f, 180.0f);
    ImGui::SliderFloat("Man x (arrow keys)", &gui_scene.man_x, -10.0f, 10.0f);
    ImGui::SliderFloat("Man y (arrow keys)", &gui_scene.man_y, -6.25f, 6.25f);

    if (gui_scene.man_phi-phi>0.0001f || gui_scene.man_phi-phi<-0.0001f)
        scene.camera.set_rotation(M_PI/2,-M_PI/2, gui_scene.man_phi*M_PI/180.0f, 0);
    scene.camera.apply_translation_orthogonal_to_screen_plane(std::cos(gui_scene.man_phi*3.141592f/180.0f)*(gui_scene.man_x-x)/2);
    scene.camera.apply_translation_orthogonal_to_screen_plane(std::sin(gui_scene.man_phi*3.141592f/180.0f)*(gui_scene.man_y-y)/2);
    scene.camera.apply_translation_in_screen_plane(-std::sin(gui_scene.man_phi*3.141592f/180.0f)*(gui_scene.man_x-x)/2,0);
    scene.camera.apply_translation_in_screen_plane(std::cos(gui_scene.man_phi*3.141592f/180.0f)*(gui_scene.man_y-y)/2,0);



}

mesh mesh_primitive_ring(float big_radius, float small_radius, const vec3& pos, const vec3& dir, size_t Nu, size_t Nv)
{
    const vec3 dir_norm = normalize(dir);
    const mat3 big_R = rotation_between_vector_mat3({0,0,1}, dir_norm);

    mesh shape;
    for( size_t ku=0; ku<Nu; ++ku ) {
        for( size_t kv=0; kv<Nv; ++kv ) {
            const float u = static_cast<float>(ku)/static_cast<float>(Nu);
            const float v = static_cast<float>(kv)/static_cast<float>(Nv);

            const float theta_u = static_cast<float>( 2* M_PI * u );
            const float theta_v = static_cast<float>( 2* M_PI * v );

            const float big_x = big_radius * std::sin(theta_u);
            const float big_y = big_radius * std::cos(theta_u);
            const vec3 dir_small = -normalize(cross({big_x, big_y, 0}, {0,0,1}));

            const float small_x = small_radius * std::sin(theta_v);
            const float small_y = small_radius * std::cos(theta_v);

            const vec3 small_p = {small_x, small_y, 0};
            const vec3 big_p = {big_x, big_y, 0};
            const mat3 small_R = rotation_between_vector_mat3({0,0,1}, dir_small);
            const vec3 p1 = small_R*small_p + big_p;
            const vec3 p = pos + big_R*p1;
            const vec3 n = normalize(rotation_from_axis_angle_mat3(dir_small, theta_v)*vec3(big_x, big_y, 0));

            shape.position.push_back( p );
            shape.normal.push_back( n );
        }
    }

    shape.connectivity = connectivity_grid(Nu, Nv, true, false);

    return shape;
}

void scene_exercise::mouse_click(scene_structure& scene, GLFWwindow* window, int, int action, int)
{
    // Mouse click is used to select a position of the control polygon
    // ******************************************************************** //

    // Window size
    int w=0;
    int h=0;
    glfwGetWindowSize(window, &w, &h);

    // Current cursor position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Convert pixel coordinates to relative screen coordinates between [-1,1]
    const float x = 2*float(xpos)/float(w)-1;
    const float y = 1-2*float(ypos)/float(h);

    // Check if shift key is pressed
    const bool key_shift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT));

    if(action==GLFW_PRESS && key_shift)
    {
        // Create the 3D ray passing by the selected point on the screen
        const ray r = picking_ray(scene.camera, x, y);

        // Check if this ray intersects a position (represented by a sphere)
        //  Loop over all positions and get the intersected position (the closest one in case of multiple intersection) const size_t N = keyframe_position.size();
        const size_t N = keyframe_position.size();
        picked_object = -1;
        float distance_min = 0.0f;
        for(size_t k=0; k<N; ++k)
        {
            const vec3 c = keyframe_position[k];
            const picking_info info = ray_intersect_sphere(r, c, 0.1f);

            if( info.picking_valid ) // the ray intersects a sphere
            {
                const float distance = norm(info.intersection-r.p); // get the closest intersection
                if( picked_object==-1 || distance<distance_min ){
                    picked_object = k;
                }
            }
        }
    }

}

void scene_exercise::mouse_move(scene_structure& scene, GLFWwindow* window)
{
    // Mouse move is used to translate a position once selected
    // ******************************************************************** //

    // Window size
    int w=0;
    int h=0;
    glfwGetWindowSize(window, &w, &h);

    // Current cursor position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Convert pixel coordinates to relative screen coordinates between [-1,1]
    const float x = 2*float(xpos)/float(w)-1;
    const float y = 1-2*float(ypos)/float(h);

    // Check that the mouse is clicked (drag and drop)
    const bool mouse_click_left  = (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT )==GLFW_PRESS);
    const bool key_shift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT));

    const size_t N = keyframe_position.size();
    if(mouse_click_left && key_shift && picked_object!=-1)
    {
        // Translate the selected object to the new pointed mouse position within the camera plane
        // ************************************************************************************** //

        // Get vector orthogonal to camera orientation
        const mat4 M = scene.camera.camera_matrix();
        const vec3 n = {M(0,2),M(1,2),M(2,2)};

        // Compute intersection between current ray and the plane orthogonal to the view direction and passing by the selected object
        const ray r = picking_ray(scene.camera, x,y);
        vec3& p0 = keyframe_position[picked_object];
        const picking_info info = ray_intersect_plane(r,n,p0);

        // translate the position
        p0 = info.intersection;

        // Make sure that duplicated positions are moved together
        int Ns = N;
        if(picked_object==0 || picked_object==Ns-3){
            keyframe_position[0] = info.intersection;
            keyframe_position[N-3] = info.intersection;
        }
        if(picked_object==1 || picked_object==Ns-2){
            keyframe_position[1] = info.intersection;
            keyframe_position[N-2] = info.intersection;
        }
        if(picked_object==2 || picked_object==Ns-1){
            keyframe_position[2] = info.intersection;
            keyframe_position[N-1] = info.intersection;
        }

    }
}

void scene_exercise::key_press(GLFWwindow* window, int key, int, int action, int, scene_structure& scene)
{
    // Button press is used to control the player
    // ******************************************************************** //
    int w=0;
    int h=0;
    glfwGetWindowSize(window, &w, &h);

    if (action == GLFW_REPEAT)
    {
        float next_pos_x = gui_scene.man_x + 0.04f*std::cos(gui_scene.man_phi*3.141592f/180.0f);
        float next_pos_y = gui_scene.man_y + 0.04f*std::sin(gui_scene.man_phi*3.141592f/180.0f);
        float last_pos_x = gui_scene.man_x - 0.04f*std::cos(gui_scene.man_phi*3.141592f/180.0f);
        float last_pos_y = gui_scene.man_y - 0.04f*std::sin(gui_scene.man_phi*3.141592f/180.0f);
        if (key == GLFW_KEY_UP && next_pos_x <= 10.0f && next_pos_x >= -10.0f && next_pos_y <= 6.25f && next_pos_y >= -6.25f)
        {
            gui_scene.man_x = next_pos_x;
            gui_scene.man_y = next_pos_y;
            scene.camera.apply_translation_orthogonal_to_screen_plane(0.02f);
        }

        if (key == GLFW_KEY_DOWN && last_pos_x <= 10.0f && last_pos_x >= -10.0f && last_pos_y <= 6.25f && last_pos_y >= -6.25f)
        {
            gui_scene.man_x = last_pos_x;
            gui_scene.man_y = last_pos_y;
            scene.camera.apply_translation_orthogonal_to_screen_plane(-0.02f);
        }

        if (key == GLFW_KEY_RIGHT)
        {
            if (gui_scene.man_phi >= -179.5f) gui_scene.man_phi -= 0.5f;
            else gui_scene.man_phi += 359.5f;
            scene.camera.set_rotation(M_PI/2,-M_PI/2, gui_scene.man_phi*M_PI/180.0f, 0);
        }
        if (key == GLFW_KEY_LEFT)
        {
            if (gui_scene.man_phi <= 179.5f) gui_scene.man_phi += 0.5f;
            else gui_scene.man_phi -= 359.5f;
            scene.camera.set_rotation(M_PI/2, -M_PI/2, gui_scene.man_phi*M_PI/180.0f, 0);
        }
        if (key == GLFW_KEY_E && gui_scene.ball_theta <= 89.5f) gui_scene.ball_theta += 0.5f;
        if (key == GLFW_KEY_D && gui_scene.ball_theta >= -89.5f) gui_scene.ball_theta -= 0.5f;
        if (key == GLFW_KEY_V && gui_scene.ball_speed <= 14.95f) gui_scene.ball_speed += 0.05f;
        if (key == GLFW_KEY_C && gui_scene.ball_speed >= 5.05f) gui_scene.ball_speed -= 0.05f;
        if (key == GLFW_KEY_ENTER) newball = true;
    }

    if (action == GLFW_PRESS)
    {
        float next_pos_x = gui_scene.man_x + 0.04f*std::cos(gui_scene.man_phi*3.141592f/180.0f);
        float next_pos_y = gui_scene.man_y + 0.04f*std::sin(gui_scene.man_phi*3.141592f/180.0f);
        float last_pos_x = gui_scene.man_x - 0.04f*std::cos(gui_scene.man_phi*3.141592f/180.0f);
        float last_pos_y = gui_scene.man_y - 0.04f*std::sin(gui_scene.man_phi*3.141592f/180.0f);
        if (key == GLFW_KEY_UP && next_pos_x <= 10.0f && next_pos_x >= -10.0f && next_pos_y <= 6.25f && next_pos_y >= -6.25f)
        {
            gui_scene.man_x = next_pos_x;
            gui_scene.man_y = next_pos_y;
            scene.camera.apply_translation_orthogonal_to_screen_plane(0.02f);
        }

        if (key == GLFW_KEY_DOWN && last_pos_x <= 10.0f && last_pos_x >= -10.0f && last_pos_y <= 6.25f && last_pos_y >= -6.25f)
        {
            gui_scene.man_x = last_pos_x;
            gui_scene.man_y = last_pos_y;
            scene.camera.apply_translation_orthogonal_to_screen_plane(-0.02f);
        }

        if (key == GLFW_KEY_RIGHT)
        {
            if (gui_scene.man_phi >= -179.5f) gui_scene.man_phi -= 0.5f;
            else gui_scene.man_phi += 359.5f;
            scene.camera.set_rotation(M_PI/2,-M_PI/2, gui_scene.man_phi*M_PI/180.0f, 0);
        }
        if (key == GLFW_KEY_LEFT)
        {
            if (gui_scene.man_phi <= 179.5f) gui_scene.man_phi += 0.5f;
            else gui_scene.man_phi -= 359.5f;
            scene.camera.set_rotation(M_PI/2, -M_PI/2, gui_scene.man_phi*M_PI/180.0f, 0);
        }
        if (key == GLFW_KEY_E && gui_scene.ball_theta <= 89.5f) gui_scene.ball_theta += 0.5f;
        if (key == GLFW_KEY_D && gui_scene.ball_theta >= -89.5f) gui_scene.ball_theta -= 0.5f;
        if (key == GLFW_KEY_V && gui_scene.ball_speed <= 15.95f) gui_scene.ball_speed += 0.05f;
        if (key == GLFW_KEY_C && gui_scene.ball_speed >= 5.05f) gui_scene.ball_speed -= 0.05f;
        if (key == GLFW_KEY_ENTER) newball = true;
    }
}

void scene_exercise::display_skybox(std::map<std::string,GLuint>& shaders, scene_structure& scene)
{
        glBindTexture(GL_TEXTURE_2D, texture_skybox);
        skybox.uniform_parameter.scaling = 150.0f;
        skybox.uniform_parameter.translation = scene.camera.camera_position() + vec3(0,0,-50.0f);
        skybox.draw(shaders["mesh"], scene.camera);
        glBindTexture(GL_TEXTURE_2D, scene.texture_white);
}

vcl::mesh create_skybox()
{
    const vec3 p000 = {-1,-1,-1};
    const vec3 p001 = {-1,-1, 1};
    const vec3 p010 = {-1, 1,-1};
    const vec3 p011 = {-1, 1, 1};
    const vec3 p100 = { 1,-1,-1};
    const vec3 p101 = { 1,-1, 1};
    const vec3 p110 = { 1, 1,-1};
    const vec3 p111 = { 1, 1, 1};

    mesh skybox;

    skybox.position = {
        p000, p100, p110, p010,
        p010, p110, p111, p011,
        p100, p110, p111, p101,
        p000, p001, p010, p011,
        p001, p101, p111, p011,
        p000, p100, p101, p001
    };

    skybox.connectivity = {
        {0,1,2}, {0,2,3}, {4,5,6}, {4,6,7},
        {8,11,10}, {8,10,9}, {17,16,19}, {17,19,18},
        {23,22,21}, {23,21,20}, {13,12,14}, {13,14,15}
    };

    const float e = 1e-3f;
    const float u0 = 0.0f;
    const float u1 = 0.25f+e;
    const float u2 = 0.5f-e;
    const float u3 = 0.75f-e;
    const float u4 = 1.0f;
    const float v0 = 0.0f;
    const float v1 = 1.0f/3.0f+e;
    const float v2 = 2.0f/3.0f-e;
    const float v3 = 1.0f;
    skybox.texture_uv = {
        {u1,v1}, {u2,v1}, {u2,v2}, {u1,v2},
        {u1,v2}, {u2,v2}, {u2,v3}, {u1,v3},
        {u2,v1}, {u2,v2}, {u3,v2}, {u3,v1},
        {u1,v1}, {u0,v1}, {u1,v2}, {u0,v2},
        {u4,v1}, {u3,v1}, {u3,v2}, {u4,v2},
        {u1,v1}, {u2,v1}, {u2,v0}, {u1,v0}
    };


    return skybox;

}



#endif

