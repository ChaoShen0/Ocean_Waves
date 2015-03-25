//
// Sam Hayhurst 2015
// Class made to be inlcuded to form the nbasic mesh of a water surface simulation
// project


#ifndef MESH_WAVE_INCLUDED
#define MESH_WAVE_INCLUDED

#include <vector>
namespace octet{
  class mesh_wave : public mesh {
  private:

    mesh *_mesh;
    material *_material;
    param_shader *_shader;
    scene_node *_node;

    // vertex structure to be passed to openGL
    struct my_vertex{
      vec3p pos;
      uint32_t colour;
    };

    // this function converts three floats into a RGBA 8 bit color
    static uint32_t make_color(float r, float g, float b) {
      return 0xff000000 + ((int)(r*255.0f) << 0) + ((int)(g*255.0f) << 8) + ((int)(b*255.0f) << 16);
    }


  public:
    // single arg constructor
    mesh_wave(){
      init();
    };

    // default destructor
    ~mesh_wave(){
      delete _mesh;
      _mesh = NULL;
    }

    // init the mesh_wave
    void init(){
      _mesh = new mesh();
      _node = new scene_node();
      _shader = new param_shader("shaders/default.vs", "shaders/simple_color.fs");
      _material = new material(vec4(1, 0, 0, 1), _shader);
    }

    // calculate and assign them to indices
    void calculate_vertices(){
      // number of steps in helix
      size_t sqr_size = 8;

      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = sqr_size * sqr_size;
      size_t num_indices = std::pow((sqr_size-1), 2) * 6;
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

        vec3 start_pos = vec3(-offset * 0.5f * sqr_size, offset * 0.5f * sqr_size, -1.0f);

        for (size_t i = 0; i != sqr_size; ++i) {
          for (size_t j = 0; j != sqr_size; ++j) {
            vtx->pos = vec3p(start_pos + vec3(offset * j, -offset * i, 0.0f));
            vtx->colour = make_color(1.0f, 1.0f, 0.0f);
            vtx++;
          }
        }

        // make the triangles
        uint32_t vn = 0;
        for (size_t i = 0; i != 55; ++i) {
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

  };
}

#endif // !MESH_WAVE_INCLUDED
