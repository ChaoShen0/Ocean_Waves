//
// Sam Hayhurst 2015
// Class made to be inlcuded to form the nbasic mesh of a water surface simulation
// project


#ifndef MESH_WAVE_INCLUDED
#define MESH_WAVE_INCLUDED

#define MY_PI  3.141592654f
#define MY_2PI 6.283185307f

#include <ctime>
#include <vector>

namespace octet{
  class mesh_wave : public mesh {
  private:

    // STRUCTURES
#pragma region STRUCTURES
    // vertex structure to be passed to openGL
    struct my_vertex{
      vec3p pos;
      uint32_t colour;
    };

    struct sine_wave{
      float amplitude;
      vec3 direction;
      float frequency;
      float omega;

      sine_wave() = default;

      // _r: rate or inverse frequency, _s speed
      sine_wave(float _a, vec3 _d, float _r, float _s){
        amplitude = _a;
        direction = _d;
        frequency = MY_2PI * _r;
        omega = _s * frequency;
      }
    };

#pragma endregion

    // MEMBER VARIABLES
#pragma region MEMBER_VARIABLES
    mesh *_mesh;
    material *_material;
    param_shader *_shader;
    scene_node *_node;
    dynarray<vec3p> points;
    dynarray<sine_wave> sine_waves;
    int num_waves = 8;
    static unsigned long long _time;
    float offset = 1.0f;
    vec3 start_pos;
    size_t sqr_size = 32;
    bool is_wireframe = false;
    float total_steepness = 0.5f;  // 0: sine wave, 1: maximum value
    random gen; // random number generator
#pragma endregion

    // PRIVATE FUNCTIONS
#pragma region PRIVATE_FUNCTIONS
    // this function converts three floats into a RGBA 8 bit color
    static uint32_t make_color(float r, float g, float b) {
      return 0xff000000 + ((int)(r*255.0f) << 0) + ((int)(g*255.0f) << 8) + ((int)(b*255.0f) << 16);
    }

    static uint32_t make_color(vec3 col) {
      return 0xff000000 + ((int)(col.x()*255.0f) << 0) + ((int)(col.y()*255.0f) << 8) + ((int)(col.z()*255.0f) << 16);
    }

    // SUPERCEEDED takes a vertex index and returns the height according to the current time
    // currently uses the default sine wave.
    float get_wave_pos(int x, int y, sine_wave wave){
      float vector_product = wave.direction.dot(vec3((float)x, (float)y, 0.0f));
      float height = wave.amplitude * sin(vector_product * wave.frequency + _time * wave.omega);
      return height;
    }

    // create default sine waves
    void create_default_sine_waves(){
      // set base wave attributes; each successive wave will have half / twice the value
      float freq = 0.01f;
      float ampl = 1.0f;
      float phase = 3.0f;
      for (size_t i = 0; i < num_waves; ++i){
        sine_waves.push_back(sine_wave(ampl * std::pow(0.5, (i + 1)),
          vec3(gen.get(-1.0f, 1.0f), gen.get(-1.0f, 1.0f), 0.0f),
          freq * std::pow(1.25f, (i + 1)), phase));
      }
    }

    // get a random colour such that the colour will be proceadurally generated
    // this version of the function doesn't take into account the vertices' position
    vec3 make_random_colour(){
      return vec3(0.0f, gen.get(0, 2) * 0.4f, 0.8f);
    }

    // Calculate mesh vertices using the Gernster Wave Function
    // Does so cumulatively over a given number of sine waves
    // this is used to update the vertices in the update loop
    // and also calculates the wave height
    vec3 compute_gerstner_points(int x, int y){
      vec3 vertex = vec3(0);

      // for each sine wave
      for (size_t i = 0; i < sine_waves.size(); ++i){
        // calculate each of the points according to Gerstner's wave function yo!
        sine_wave wave = sine_waves[i];
        float steepness = total_steepness / (wave.omega * sine_waves.size());
        float radians = wave.omega * wave.direction.dot(vec3(x, y, 0.0f) + _time * wave.omega);
        float x_pos = steepness * wave.direction.x() * cosf(radians);
        float y_pos = steepness * wave.direction.y() * cosf(radians);
        float z_pos = wave.amplitude * sinf(radians);
        vertex += vec3(x_pos, y_pos, z_pos);
      }
      return vertex;
    }

#pragma endregion

  public:
    // single arg constructor
    mesh_wave(){
      init();
    };

    // default destructor
    ~mesh_wave(){
      delete _mesh;
      delete _material;
      delete _shader;
      delete _node;
      _node = NULL;
      _shader = NULL;
      _material = NULL;
      _mesh = NULL;
      points.reset();
    }

    // init the mesh_wave
    void init(){
      _mesh = new mesh();
      _node = new scene_node();
      _shader = new param_shader("shaders/default.vs", "shaders/simple_color.fs");
      _material = new material(vec4(1, 0, 0, 1), _shader);

      start_pos = vec3(-offset * 0.5f * sqr_size, offset * 0.5f * sqr_size, -1.0f);
      points.resize(sqr_size * sqr_size);

      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = sqr_size * sqr_size;
      size_t num_indices = (sqr_size - 1) * (sqr_size - 1) * 6;
      _mesh->allocate(sizeof(my_vertex) * num_vertices, sizeof(uint32_t) * num_indices);
      _mesh->set_params(sizeof(my_vertex), num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_INT);

      // describe the structure of my_vertex to OpenGL
      _mesh->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      _mesh->add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, 12, GL_TRUE);

      // init random number generator
      gen.set_seed(time(0));

      // construct the default sine waves
      create_default_sine_waves();
    }

    // calculate and assign them to indices
    void rebuild_mesh(){
      // these write-only locks give access to the vertices and indices.
      // they will be released at the next } (the end of the scope)
      gl_resource::wolock vl(_mesh->get_vertices());
      my_vertex *vtx = (my_vertex *)vl.u8();
      gl_resource::wolock il(_mesh->get_indices());
      uint32_t *idx = il.u32();


      // make the vertices
      for (size_t i = 0; i != sqr_size; ++i) {
        for (size_t j = 0; j != sqr_size; ++j) {
          vtx->pos = points[j + i*sqr_size];
          vtx->colour = make_color(make_random_colour());
          vtx++;
        }
      }

      // make the triangles
      uint32_t vn = 0;
      for (size_t i = 0; i != sqr_size * (sqr_size - 1) + 1; ++i) {
        // 0--2
        // | \|
        // 1--3
        if (i % sqr_size != sqr_size - 1){
          idx[0] = i;
          idx[1] = i + sqr_size + 1;
          idx[2] = i + 1;
          idx[3] = i;
          idx[4] = i + sqr_size;
          idx[5] = i + sqr_size + 1;
          idx += 6;
        }
      }

    }

    // add the mesh to the scene
    void add_to_the_scene(visual_scene *app_scene){
      app_scene->add_mesh_instance(new mesh_instance(_node, _mesh, _material));
    }

    // update function, recalculates the nodes to change the mesh ideally to look like water
    void update(){
      for (size_t i = 0; i != sqr_size; ++i) {
        for (size_t j = 0; j != sqr_size; ++j) {
          vec3 wave_vector = compute_gerstner_points(j, i);
          points[j + i*sqr_size] = vec3p(start_pos + vec3(offset * j, -offset * i, 0.0f) + wave_vector);
        }
      }
      ++_time;
    }

    // GETS & SETS
#pragma region GET_&_SETS
    // Get & Set functions
    scene_node *get_node() { return _node; }

    void inline increment_amplitude() {
      sine_waves.begin()->amplitude += 0.1f;
    }

    void inline decrement_amplitude() {
      sine_waves.begin()->amplitude -= 0.1f;
    }

    void inline increment_frequency() {
      sine_waves.begin()->frequency += MY_2PI * 10.1f;
    }

    void inline decrement_frequency(){
      sine_waves.begin()->frequency -= MY_2PI * 0.01f;
    }

    void inline increment_direction(){
      sine_waves.begin()->direction = sine_waves.begin()->direction * mat4t().rotateZ(15.0f);
    }

    void inline decrement_direction(){
      sine_waves.begin()->direction = sine_waves.begin()->direction * mat4t().rotateZ(15.0f);
    }

    void inline wire_frame_mode(){
      size_t num_vertices = sqr_size * sqr_size;
      size_t num_indices = (sqr_size - 1) * (sqr_size - 1) * 6;
      if (!is_wireframe){
        _mesh->set_params(sizeof(my_vertex), num_indices, num_vertices, GL_LINES, GL_UNSIGNED_INT);
      }
      else{
        _mesh->set_params(sizeof(my_vertex), num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_INT);
      }
      is_wireframe = !is_wireframe;
    }

#pragma endregion

  };

  unsigned long long mesh_wave::_time = 0;

}



#endif // !MESH_WAVE_INCLUDED
