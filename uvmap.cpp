#pragma leco tool
#pragma leco add_shader "uvmap.vert"
#pragma leco add_shader "uvmap.frag"

import dotz;
import hai;
import jojo;
import jute;
import sires;
import sitime;
import stubby;
import traits;
import vee;
import voo;
import wavefront;

using wavefront::vtx;

int main() {
  auto i = vee::create_instance("wavefront-uvmap");
  auto dbg = vee::create_debug_utils_messenger();
  auto [pd, qf] = vee::find_physical_device_with_universal_queue(nullptr);
  auto d = vee::create_single_queue_device(pd, qf, {
    .fillModeNonSolid = true,
  });
  auto q = voo::queue { qf };

  voo::single_cb cb { qf };

  auto [v_buf, v_count] = wavefront::load_model(pd, "model.obj");

  vee::extent ext { 1024, 1024 };
  voo::offscreen::buffers ofs { pd, ext, VK_FORMAT_R8G8B8A8_SRGB };

  auto pl = vee::create_pipeline_layout();
  auto gp = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = ofs.render_pass(),
    .polygon_mode = VK_POLYGON_MODE_LINE,
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

  {
    voo::cmd_buf_one_time_submit pcb { cb.cb() };
    v_buf.setup_copy(*pcb);

    {
      voo::cmd_render_pass scb { ofs.render_pass_begin({
        .command_buffer = *pcb,
        .clear_colours { vee::clear_colour({1,1,1,1}) },
      })};
      vee::cmd_set_viewport(*pcb, ext);
      vee::cmd_set_scissor(*pcb, ext);
      vee::cmd_bind_vertex_buffers(*pcb, 0, v_buf.local_buffer());
      vee::cmd_bind_gr_pipeline(*pcb, *gp);
      vee::cmd_draw(*pcb, v_count);
    }
    ofs.cmd_copy_to_host(*pcb);
  }
  q.queue_submit({ .command_buffer = cb.cb() });
  vee::device_wait_idle();

  auto m = ofs.map_host();
  auto mm = static_cast<stbi::pixel *>(*m);
  stbi::write_rgba_unsafe("out/model.png", ext.width, ext.height, mm);
}
