#pragma once

#include "../../exercises/base_exercise/base_exercise.hpp"
#include <math.h>

#ifdef PROJET

#define num_grass 50
#define num_string 20

using namespace vcl;

// Stores some parameters that can be set from the GUI
struct gui_scene_structure
{
    bool wireframe;
    float scaling = 3.0f;
    int octave = 7;
    float persistency = 0.4f;

    float ball_speed = 8.0f;
    float ball_theta = 45.0f;
    float man_phi = 0.0f;
    float man_x = 0.0f;
    float man_y = 0.0f;
};

struct particle_element
{
    vec3 p; // Position
    vec3 v; // Speed
    bool previous_collision_ring;
    bool previous_collision_stand;
};

struct scene_exercise : base_scene_exercise
{

    /** A part must define two functions that are called from the main function:
     * setup_data: called once to setup data before starting the animation loop
     * frame_draw: called at every displayed frame within the animation loop
     *
     * These two functions receive the following parameters
     * - shaders: A set of shaders.
     * - scene: Contains general common object to define the 3D scene. Contains in particular the camera.
     * - data: The part-specific data structure defined previously
     * - gui: The GUI structure allowing to create/display buttons to interact with the scene.
    */

    void setup_data(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui);
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui);
    void display_stands(std::map<std::string,GLuint>& shaders, scene_structure& scene);
    void display_net(std::map<std::string,GLuint>& shaders, scene_structure& scene);

    void set_gui(scene_structure& scene);

    mesh_drawable_hierarchy create_stand();

    // visual representation of a surface
    mesh_drawable terrain;
    mesh_drawable_hierarchy stand[2];
    mesh_drawable grass[num_grass];

    GLuint texture_terrain_id;
    GLuint texture_grass_id;
    GLuint texture_board_id;

    // Called every time the mouse is clicked
    void mouse_click(scene_structure& scene, GLFWwindow* window, int button, int action, int mods);
    // Called every time the mouse is moved
    void mouse_move(scene_structure& scene, GLFWwindow* window);
    // Called every time buttons are pressed
    void key_press(GLFWwindow* window, int key, int scancode, int action, int mod, scene_structure& scene);

    // Data (p_i,t_i)
    std::vector<vec3> keyframe_position; // Given positions
    std::vector<float> keyframe_time;         // Time at given positions

    // Store the index of a selected sphere
    int picked_object;

    gui_scene_structure gui_scene;

    mesh_drawable_hierarchy man;
    mesh_drawable ball;
    void display_man(std::map<std::string,GLuint>& shaders, scene_structure& scene);
    void display_ball(std::map<std::string,GLuint>& shaders, scene_structure& scene);

    std::list<particle_element> ball_particles;
    timer_event period;


    // Basketball Stand Parameters
    // ************************************************ //

    const vec3 stand_position1 = {-9.5f, 0, 0};
    const vec3 stand_position2 = {9.5f, 0, 0};

    particle_element pA[2*num_string];
    particle_element pB[2*num_string];
    particle_element pC[2*num_string];
    particle_element pD[2*num_string];
    
    const float stand_radius = 0.09f;
    const float stand_height = 1.8f;
    const float board_width = 1.0f;
    const float board_length = 1.6f;
    const float ring_radius = board_width/3.5f;
    const float connection_width = board_width/10.0f;
    const float connection_length = board_length/5.0f;
    const float ring_board_gap = board_width/6.0f;
    
    const vec3 board_position1 = stand_position1 + vec3(0,0,stand_height) + rotation_from_axis_angle_mat3({0,1,0}, M_PI/6.0f) * (vec3(0,0,stand_height/3.0f) + rotation_from_axis_angle_mat3({0,1,0}, M_PI/3.0f) * vec3(0,0,stand_height*1.05f/5.0f));
    const vec3 board_position2 = stand_position2 + vec3(0,0,stand_height) + rotation_from_axis_angle_mat3({0,1,0}, -M_PI/6.0f) * (vec3(0,0,stand_height/3.0f) + rotation_from_axis_angle_mat3({0,1,0}, -M_PI/3.0f) * vec3(0,0,stand_height*1.05f/5.0f));

    const vec3 ring_position1 = stand_position1 + vec3(0,0,stand_height) + rotation_from_axis_angle_mat3({0,1,0}, M_PI/6.0f) * (vec3(0,0,stand_height/3.0f) + rotation_from_axis_angle_mat3({0,1,0}, M_PI/3.0f) * (vec3(0,0,stand_height*1.05f/5.0f) + vec3(ring_board_gap,0,ring_radius+connection_width)));
    const vec3 ring_position2 = stand_position2 + vec3(0,0,stand_height) + rotation_from_axis_angle_mat3({0,1,0}, -M_PI/6.0f) * (vec3(0,0,stand_height/3.0f) + rotation_from_axis_angle_mat3({0,1,0}, -M_PI/3.0f) * (vec3(0,0,stand_height*1.05f/5.0f) + vec3(-ring_board_gap,0,ring_radius+connection_width)));

    const vec3 join_ball1_position1 = stand_position1 + vec3(0,0,stand_height);
    const vec3 join_ball2_position1 = stand_position1 + vec3(0,0,stand_height) + rotation_from_axis_angle_mat3({0,1,0}, M_PI/6.0f) * vec3(0,0,stand_height/3.0f);
    const vec3 join_ball1_position2 = stand_position2 + vec3(0,0,stand_height);
    const vec3 join_ball2_position2 = stand_position2 + vec3(0,0,stand_height) + rotation_from_axis_angle_mat3({0,1,0}, -M_PI/6.0f) * vec3(0,0,stand_height/3.0f);

    // Basketball Net Parameters
    // ************************************************ //

    // Vertical rest lengths
    const float L_AB = 0.1f;
    const float L_BC = 0.1f;
    const float L_CD = 0.1f;
    // Horizontal rest lengths
    const float L_horizontal_B = ring_radius*std::sin(M_PI/num_string)*0.6f;
    const float L_horizontal_C = ring_radius*std::sin(M_PI/num_string)*0.4f;
    const float L_horizontal_D = ring_radius*std::sin(M_PI/num_string)*0.3f;
    
    const float m = 0.001f;        // particle mass
    const float K = 1.0f;          // spring stiffness         
    const float mu_dampling = 0.01f;       // damping coefficient

    const float rebound_coefficient = 0.85f;
    const float mu_air = 0.0003f;
    const float gravity = 9.81f; // gravity
    const vec3 f_weight = {0, 0, -m*gravity};

    const float ball_radius = 0.15f;
    bool newball = false;
    float timesincenewball = -5.0f;

    mesh_drawable sphere;
    segment_drawable_immediate_mode segment_drawer;

    // Skybox
    // ************************************************ //

    mesh_drawable skybox;
    GLuint texture_skybox;
    void display_skybox(std::map<std::string,GLuint>& shaders, scene_structure& scene);

    timer_event timer;

    // Collision stuffs
    // ************************************************ //
    mesh ring1;
    mesh ring2;
    mesh pillar1;
    mesh pillar2;
    mesh join_ball1_1;
    mesh join_ball2_1;
    mesh join_ball1_2;
    mesh join_ball2_2;
    mesh connection1_1;
    mesh connection2_1;
    mesh connection1_2;
    mesh connection2_2;

};

#endif


