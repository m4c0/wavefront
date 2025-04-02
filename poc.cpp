#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"
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

static void load_model(voo::h2l_buffer & buf, auto & vs) {
  voo::mapmem mm { buf.host_memory() };
  auto * m = static_cast<wavefront::vtx *>(*mm);
  for (auto v : vs) *m++ = v;
}

static auto create_vbuffer(vee::physical_device pd) {
  struct {
    voo::h2l_buffer buffer;
    unsigned count;
  } res;
  auto vs = wavefront::read_model("model.obj");

  unsigned v_count = res.count = vs.size();
  unsigned v_size = v_count * sizeof(wavefront::vtx);
  res.buffer = voo::h2l_buffer { pd, v_size };
  load_model(res.buffer, vs);
  return res;
}

using wavefront::vtx;

struct lng : public vapp {
  void run() override {
    main_loop("poc-voo", [&](auto & dq, auto & sw) {
      auto [v_buf, v_count] = create_vbuffer(dq.physical_device());

      auto pl = vee::create_pipeline_layout({
        vee::vertex_push_constant_range<float>()
      });
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
          vee::vertex_attribute_vec2(0, traits::offset_of(&vtx::txt)),
          vee::vertex_attribute_vec3(0, traits::offset_of(&vtx::nrm)),
        },
      });

      bool loaded = false;
      sitime::stopwatch t {};
      extent_loop(dq.queue(), sw, [&] {
        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          if (!loaded) {
            v_buf.setup_copy(*pcb);
            loaded = true;
          }
          float ms = t.millis() / 1000.0;

          auto scb = sw.cmd_render_pass({ *pcb });
          vee::cmd_set_viewport(*pcb, sw.extent());
          vee::cmd_set_scissor(*pcb, sw.extent());
          vee::cmd_bind_vertex_buffers(*pcb, 0, v_buf.local_buffer());
          vee::cmd_bind_gr_pipeline(*pcb, *gp);
          vee::cmd_push_vertex_constants(*pcb, *pl, &ms);
          vee::cmd_draw(*pcb, v_count);
        });
      });
    });
  }
} t;
