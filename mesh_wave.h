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
      size_t num_steps = 320;

      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = num_steps * 2 + 2;
      size_t num_indices = num_steps * 6;
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
        float radius1 = 1.0f;
        float radius2 = 7.0f;
        float height = 24.0f;
        float num_twists = 4.0f;
        for (size_t i = 0; i != num_steps + 1; ++i) {
          float r = 0.0f, g = 1.0f * i / num_steps, b = 1.0f;
          float y = i * (height / num_steps) - height * 0.5f;
          float angle = i * (num_twists * 2.0f * 3.14159265f / num_steps);
          vtx->pos = vec3p(cosf(angle) * radius1, y, sinf(angle) * radius1);
          vtx->colour = make_color(r, g, b);
          log("%f %f %f %08x\n", r, g, b, vtx->colour);
          vtx++;
          vtx->pos = vec3p(cosf(angle) * radius2, y, sinf(angle) * radius2);
          vtx->colour = make_color(r, g, b);
          vtx++;
        }

        // make the triangles
        uint32_t vn = 0;
        for (size_t i = 0; i != num_steps; ++i) {
          // 0--2
          // | \|
          // 1--3
          idx[0] = vn + 0;
          idx[1] = vn + 3;
          idx[2] = vn + 1;
          idx += 3;

          idx[0] = vn + 0;
          idx[1] = vn + 2;
          idx[2] = vn + 3;
          idx += 3;

          vn += 2;
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
