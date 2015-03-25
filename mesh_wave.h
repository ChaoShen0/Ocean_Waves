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
    mesh_wave(){
      init();
    };

    ~mesh_wave(){
      delete _mesh;
      _mesh = NULL;
    }

    void init(){
      _mesh = new mesh();
    }

  };
}

#endif // !MESH_WAVE_INCLUDED
