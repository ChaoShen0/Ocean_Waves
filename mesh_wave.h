//
// Sam Hayhurst 2015
// Class made to be inlcuded to form the nbasic mesh of a water surface simulation
// project


#ifndef MESH_WAVE_INCLUDED
#define MESH_WAVE_INCLUDED

#define MY_PI  3.141592654f
#define MY_2PI 6.283185307f


#include <vector>
namespace octet{
  class mesh_wave : public mesh {
  private:

    mesh *_mesh;
    material *_material;
    param_shader *_shader;
    scene_node *_node;
    dynarray<vec3p> points;

    // vertex structure to be passed to openGL
    struct my_vertex{
      vec3p pos;
      uint32_t colour;
    };

    static unsigned int _time;
    float offset;
    vec3 start_pos;
    size_t sqr_size;

    // this function converts three floats into a RGBA 8 bit color
    static uint32_t make_color(float r, float g, float b) {
      return 0xff000000 + ((int)(r*255.0f) << 0) + ((int)(g*255.0f) << 8) + ((int)(b*255.0f) << 16);
    }

    // default sine wave
    float amplitude = 1.0f;
    vec3 direction = vec3(1.0f, 1.0f, 0.0f);
    float frequency = MY_2PI / 90.0f;   // multiply by inverse wavelength
    float omega = 3.0f * frequency;           // phase velocity

    // takes a vertex index and returns the height according to the current time
    // currently uses the default sine wave.
    float get_wave_pos(int x, int y){
      float vector_product = direction.dot(vec3((float) x,(float) y, 0.0f));
      float height = amplitude * sin(vector_product * frequency + _time * omega);

      return height;
    }

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

      offset = 1.0f;
      sqr_size = 16;
      start_pos = vec3(-offset * 0.5f * sqr_size, offset * 0.5f * sqr_size, -1.0f);
      points.resize(sqr_size * sqr_size);
    }

    // calculate and assign them to indices
    void rebuild_mesh(){
      _mesh->init();

      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = sqr_size * sqr_size;
      size_t num_indices = (sqr_size - 1) * (sqr_size - 1) * 6;
      _mesh->allocate(sizeof(my_vertex) * num_vertices, sizeof(uint32_t) * num_indices);
      _mesh->set_params(sizeof(my_vertex), num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_INT);

      // describe the structure of my_vertex to OpenGL
      _mesh->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      _mesh->add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, 12, GL_TRUE);

      {
        // these write-only locks give access to the vertices and indices.
        // they will be released at the next } (the end of the scope)
        gl_resource::wolock vl(_mesh->get_vertices());
        my_vertex *vtx = (my_vertex *)vl.u8();
        gl_resource::wolock il(_mesh->get_indices());
        uint32_t *idx = il.u32();

        // make the vertices
        float offset = 1.0f; // offset per vertex

        for (size_t i = 0; i != sqr_size; ++i) {
          for (size_t j = 0; j != sqr_size; ++j) {
            vtx->pos = points[j + i*sqr_size];
            vtx->colour = make_color(1.0f * j / sqr_size, 1.0f * i / sqr_size, 0.0f);
            vtx++;
          }
        }

        // make the triangles
        uint32_t vn = 0;
        for (size_t i = 0; i !=  sqr_size * (sqr_size - 1) + 1; ++i) {
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
    }

    // add the mesh to the scene
    void add_to_the_scene(visual_scene *app_scene){
      app_scene->add_mesh_instance(new mesh_instance(_node, _mesh, _material));
    }

    // update function, recalculates the nodes to change the mesh ideally to look like water
    void update(){

      for (size_t i = 0; i != sqr_size; ++i) {
        for (size_t j = 0; j != sqr_size; ++j) {
          points[j + i*sqr_size] = vec3p(start_pos + vec3(offset * j, -offset * i, 0.0f) + vec3(0, 0, get_wave_pos(j, i)));
        }
      }
      ++_time;
    }

#pragma region GET_&_SETS
    // Get & Set functions
    scene_node *get_node() { return _node; }
#pragma endregion 

  };


  unsigned int mesh_wave::_time = 0;

}



#endif // !MESH_WAVE_INCLUDED
