#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"
#pragma leco add_resource "model.obj"

import dotz;
import jojo;
import jute;
import sires;
import sitime;
import traits;
import vee;
import voo;
import vapp;

struct vtx {
  dotz::vec3 pos;
  dotz::vec2 txt;
};

struct invalid_number {};
static constexpr int atoi(jute::view v) {
  int res = 0;
  for (auto c : v) {
    if (c < '0' || c > '9') throw invalid_number {};
    res = res * 10 + (c - '0');
  }
  return res;
}
static constexpr float atof(jute::view v) {
  float res = 0;
  int decimals = -1;
  bool negative = v[0] == '-';
  if (negative) v = v.subview(1).after;
  for (auto c : v) {
    if (c == '.' && decimals == -1) {
      decimals = 0;
      continue;
    } else if (c == '.') throw invalid_number {};
    if (c < '0' || c > '9') throw invalid_number {};

    res = res * 10 + (c - '0');

    if (decimals >= 0) decimals++;
  }
  for (auto i = 0; i < decimals; i++) res /= 10;
  return negative ? -res : res;
}

static auto count_points() {
  struct {
    unsigned v_count {};
    unsigned x_count {};
  } res {};

  jojo::readlines(sires::real_path_name("model.obj"), [&](auto line) {
    if (line.size() < 2) return;
    if (line[0] == 'v' && line[1] == ' ') res.v_count++;
    if (line[0] == 'f' && line[1] == ' ') {
      res.x_count += 3;
      if (line.subview(1).after
          .split(' ').after
          .split(' ').after
          .split(' ').after
          .size()) res.x_count += 3;
    }
  });

  return res;
}

static void load_model(voo::h2l_buffer & buf) {
  voo::mapmem mm { buf.host_memory() };
  auto * mv = static_cast<vtx *>(*mm);
  auto * mt = static_cast<vtx *>(*mm);

  jojo::readlines(sires::real_path_name("model.obj"), [&](auto line) {
    auto [cmd, vr] = line.split(' ');
    auto [x, xr] = vr.split(' ');
    auto [y, yr] = xr.split(' ');
    if (cmd == "v") {
      auto [z, zr] = yr.split(' ');
      dotz::vec3 p { atof(x), atof(y), atof(z) };
      (*mv++).pos = p * 100.0;
    } else if (cmd == "vt") {
      (*mt++).txt = { atof(x), atof(y) };
    }
  });
}

static void load_indices(voo::h2l_buffer & buf) {
  unsigned count {};
  voo::memiter<unsigned> m { buf.host_memory(), &count };

  jojo::readlines(sires::real_path_name("model.obj"), [&](auto line) {
    if (line[0] != 'f' || line[1] != ' ') return;
    auto [f, fr] = line.split(' ');
    auto [v0, v0r] = fr.split(' ');
    auto [v1, v1r] = v0r.split(' ');
    auto [v2, v2r] = v1r.split(' ');
    auto [v3, v3r] = v2r.split(' ');

    auto i0 = atoi(v0.split('/').before);
    auto i1 = atoi(v1.split('/').before);
    auto i2 = atoi(v2.split('/').before);
    auto i3 = atoi(v3.split('/').before);
    m += i0 - 1U; m += i1 - 1U; m += i2 - 1U;
    if (i3) m += i3 - 1U; m += i0 - 1U; m += i2 - 1U;
  });
}

struct lng : public vapp {
  void run() override {
    main_loop("poc-voo", [&](auto & dq, auto & sw) {
      auto [v_count, x_count] = count_points();

      unsigned v_size = v_count * sizeof(vtx);
      voo::h2l_buffer v_buf { dq.physical_device(), v_size };
      load_model(v_buf);

      unsigned x_size = x_count * sizeof(unsigned);
      voo::h2l_buffer x_buf {
        dq.physical_device(), x_size,
        vee::buffer_usage::index_buffer
      };
      load_indices(x_buf);

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
        },
      });

      bool loaded = false;
      sitime::stopwatch t {};
      extent_loop(dq.queue(), sw, [&] {
        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          if (!loaded) {
            v_buf.setup_copy(*pcb);
            x_buf.setup_copy(*pcb);
            loaded = true;
          }
          float ms = t.millis() / 1000.0;

          auto scb = sw.cmd_render_pass({ *pcb });
          vee::cmd_set_viewport(*pcb, sw.extent());
          vee::cmd_set_scissor(*pcb, sw.extent());
          vee::cmd_bind_vertex_buffers(*pcb, 0, v_buf.local_buffer());
          vee::cmd_bind_index_buffer_u32(*pcb, x_buf.local_buffer());
          vee::cmd_bind_gr_pipeline(*pcb, *gp);
          vee::cmd_push_vertex_constants(*pcb, *pl, &ms);
          vee::cmd_draw_indexed(*pcb, x_count);
        });
      });
    });
  }
} t;
