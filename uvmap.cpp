#pragma leco app
#pragma leco add_shader "uvmap.vert"
#pragma leco add_shader "uvmap.frag"
#pragma leco add_resource "model.obj"

import dotz;
import hai;
import jojo;
import jute;
import sires;
import sitime;
import traits;
import vee;
import voo;
import vapp;
import wavefront;

using wavefront::vtx;

struct lng : public vapp {
  void run() override {
    main_loop("poc-voo", [&](auto & dq, auto & sw) {
      auto [v_buf, v_count] = wavefront::load_model(dq.physical_device(), "model.obj");

      auto pl = vee::create_pipeline_layout();
      auto gp = vee::create_graphics_pipeline({
        .pipeline_layout = *pl,
        .render_pass = dq.render_pass(),
        .back_face_cull = false,
        .shaders {
          voo::shader("uvmap.vert.spv").pipeline_vert_stage(),
          voo::shader("uvmap.frag.spv").pipeline_frag_stage(),
        },
        .bindings {
          vee::vertex_input_bind(sizeof(vtx)),
        },
        .attributes {
          vee::vertex_attribute_vec2(0, traits::offset_of(&vtx::txt)),
        },
      });

      bool loaded = false;
      extent_loop(dq.queue(), sw, [&] {
        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          if (!loaded) {
            v_buf.setup_copy(*pcb);
            loaded = true;
          }

          auto scb = sw.cmd_render_pass({ *pcb });
          vee::cmd_set_viewport(*pcb, sw.extent());
          vee::cmd_set_scissor(*pcb, sw.extent());
          vee::cmd_bind_vertex_buffers(*pcb, 0, v_buf.local_buffer());
          vee::cmd_bind_gr_pipeline(*pcb, *gp);
          vee::cmd_draw(*pcb, v_count);
        });
      });
    });
  }
} t;
