module wavefront;
import jojo;
import jute;
import hai;
import silog;

struct invalid_number {};

static wavefront::vtx read_vertex(auto & pos, auto & txt, auto & nrm, jute::view vtn) {
  auto [v, vr] = vtn.split('/');
  auto [t, tr] = vr.split('/');
  auto [n, nr] = tr.split('/');
  
  return wavefront::vtx {
    .pos = pos.seek(jute::to_u32(v) - 1),
    .txt = txt.seek(jute::to_u32(t) - 1),
    .nrm = nrm.seek(jute::to_u32(n) - 1),
  };
}
hai::chain<wavefront::vtx> wavefront::read_model(jute::view model) {
  hai::chain<dotz::vec3> pos { 10240 };
  hai::chain<dotz::vec2> txt { 10240 };
  hai::chain<dotz::vec3> nrm { 10240 };
  hai::chain<wavefront::vtx> vtx { 102400 };

  jojo::readlines(model, [&](auto line) {
    if (line.size() < 2) return;
    auto [cmd, vr] = line.split(' ');
    auto [x, xr] = vr.split(' ');
    auto [y, yr] = xr.split(' ');
    if (cmd == "v") {
      auto [z, zr] = yr.split(' ');
      dotz::vec3 p { jute::to_f(x), jute::to_f(y), jute::to_f(z) };
      pos.push_back(p);
    } else if (cmd == "vn") {
      auto [z, zr] = yr.split(' ');
      dotz::vec3 n { jute::to_f(x), jute::to_f(y), jute::to_f(z) };
      nrm.push_back(n);
    } else if (cmd == "vt") {
      txt.push_back({ jute::to_f(x), jute::to_f(y) });
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
      vtx.push_back(v2);
      vtx.push_back(v3);
    }
  });

  return vtx;
}

static void load_model(voo::h2l_buffer & buf, auto & vs) {
  voo::mapmem mm { buf.host_memory() };
  auto * m = static_cast<wavefront::vtx *>(*mm);
  for (auto v : vs) *m++ = v;
}

wavefront::model_pair wavefront::load_model(vee::physical_device pd, jute::view model) {
  struct model_pair res {};
  auto vs = wavefront::read_model(model);

  unsigned v_count = res.v_count = vs.size();
  unsigned v_size = v_count * sizeof(wavefront::vtx);
  res.v_buffer = voo::h2l_buffer { pd, v_size };
  ::load_model(res.v_buffer, vs);
  silog::log(silog::info, "Loaded model %.*s", static_cast<unsigned>(model.size()), model.begin());
  return res;
}

