#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import dotz;
import traits;
import vee;
import voo;
import vapp;

struct vtx {
  dotz::vec3 pos;
};

static unsigned load_model(voo::h2l_buffer & buf) {
  unsigned count {};
  voo::memiter<vtx> m { buf.host_memory(), &count };
  m += { .pos { -1, -1, 0 } };
  m += { .pos { +1, +1, 0 } };
  m += { .pos { +1, -1, 0 } };

  m += { .pos { -1, -1, 0 } };
  m += { .pos { +1, +1, 0 } };
  m += { .pos { -1, +1, 0 } };
  return count;
}

struct lng : public vapp {
  void run() override {
    main_loop("poc-voo", [&](auto & dq, auto & sw) {
      voo::h2l_buffer buf { dq.physical_device(), sizeof(vtx) * 1024 };
      int count = load_model(buf);

      auto pl = vee::create_pipeline_layout();
      auto gp = vee::create_graphics_pipeline({
        .pipeline_layout = *pl,
        .render_pass = dq.render_pass(),
        .back_face_cull = false,
        .shaders {
          voo::shader("poc.vert.spv").pipeline_vert_stage(),
          voo::shader("poc.frag.spv").pipeline_frag_stage(),
        },
        .bindings {
          vee::vertex_input_bind(sizeof(vtx)),
        },
        .attributes {
          vee::vertex_attribute_vec3(0, traits::offset_of(&vtx::pos)),
        },
      });

      bool loaded = false;
      extent_loop(dq.queue(), sw, [&] {
        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          if (!loaded) {
            buf.setup_copy(*pcb);
            loaded = true;
          }

          auto scb = sw.cmd_render_pass({ *pcb });
          vee::cmd_set_viewport(*pcb, sw.extent());
          vee::cmd_set_scissor(*pcb, sw.extent());
          vee::cmd_bind_vertex_buffers(*pcb, 0, buf.local_buffer());
          vee::cmd_bind_gr_pipeline(*pcb, *gp);
          vee::cmd_draw(*pcb, count);
        });
      });
    });
  }
} t;
