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

struct vtx {
  dotz::vec3 pos;
  dotz::vec2 txt;
  dotz::vec3 nrm;
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

static vtx read_vertex(auto & pos, auto & txt, auto & nrm, jute::view vtn) {
  auto [v, vr] = vtn.split('/');
  auto [t, tr] = vr.split('/');
  auto [n, nr] = tr.split('/');
  
  return vtx {
    .pos = pos.seek(atoi(v) - 1),
    .txt = txt.seek(atoi(t) - 1),
    .nrm = nrm.seek(atoi(n) - 1),
  };
}
static auto read_model() {
  hai::chain<dotz::vec3> pos { 10240 };
  hai::chain<dotz::vec2> txt { 10240 };
  hai::chain<dotz::vec3> nrm { 10240 };
  hai::chain<vtx> vtx { 102400 };

  jojo::readlines(sires::real_path_name("model.obj"), [&](auto line) {
    if (line.size() < 2) return;
    auto [cmd, vr] = line.split(' ');
    auto [x, xr] = vr.split(' ');
    auto [y, yr] = xr.split(' ');
    if (cmd == "v") {
      auto [z, zr] = yr.split(' ');
      dotz::vec3 p { atof(x), atof(y), atof(z) };
      pos.push_back(p * 100.0);
    } else if (cmd == "vn") {
      auto [z, zr] = yr.split(' ');
      dotz::vec3 n { atof(x), atof(y), atof(z) };
      nrm.push_back(n);
    } else if (cmd == "vt") {
      txt.push_back({ atof(x), atof(y) });
    } else if (cmd == "f") {
      auto [z, zr] = yr.split(' ');
      auto [w, wr] = zr.split(' ');
      auto v0 = read_vertex(pos, txt, nrm, x);
      auto v1 = read_vertex(pos, txt, nrm, y);
      auto v2 = read_vertex(pos, txt, nrm, z);
      vtx.push_back(v0);
      vtx.push_back(v1);
      vtx.push_back(v2);
      if (!w.size()) return;
      auto v3 = read_vertex(pos, txt, nrm, w);
      vtx.push_back(v0);
      vtx.push_back(v3);
      vtx.push_back(v2);
    }
  });

  return vtx;
}

static void load_model(voo::h2l_buffer & buf, auto & vs) {
  voo::mapmem mm { buf.host_memory() };
  auto * m = static_cast<vtx *>(*mm);
  for (auto v : vs) *m++ = v;
}

struct lng : public vapp {
  void run() override {
    main_loop("poc-voo", [&](auto & dq, auto & sw) {
      auto vs = read_model();

      unsigned v_count = vs.size();
      unsigned v_size = v_count * sizeof(vtx);
      voo::h2l_buffer v_buf { dq.physical_device(), v_size };
      load_model(v_buf, vs);

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
