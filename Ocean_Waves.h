///////////////////////////////////////f////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include "mesh_wave.h"

namespace octet {
  /// Scene containing a box with octet.
  class Ocean_Waves : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;
    ref<camera_instance> camera;
    ref<mesh_wave> wave;


    void user_controls(){
      if (is_key_going_down('O')){ wave->increment_frequency(); }

      if (is_key_going_down('P')){ wave->decrement_frequency(); }

      if (is_key_going_down('K')){ wave->increment_direction(); }

      if (is_key_going_down('L')){ wave->decrement_direction(); }

      if (is_key_going_down('W')){ wave->wire_frame_mode(); }
    }

  public:
    /// this is called when we construct the class before everything is initialised.
    Ocean_Waves(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      // mess round with the camera
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      camera = app_scene->get_camera_instance(0);
      camera->get_node()->translate(vec3(0.0f, 0.0f, 4.0f));

      wave = new mesh_wave();
      wave->update();
      wave->rebuild_mesh();
      wave->add_to_the_scene(app_scene);

      scene_node *node = wave->get_node();
      node->rotate(-45.0f, vec3(1.0f, 0.0f, 0.0f));
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.

      wave->update();
      wave->rebuild_mesh();

      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

      // tumble the box  (there is only one mesh instance)
      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      //node->rotate(0.5f, vec3(0, 1, 0));

      user_controls();
    }
  };
}
